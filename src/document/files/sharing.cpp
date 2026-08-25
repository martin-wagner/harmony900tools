// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>
#include <QFileDialog>
#include <fstream>
#include <nlohmann/json.hpp>

#include "lib/uid.h"
#include "document/data/data.h"
#include "document/config.h"
#include "document/data/catalogue.h"
#include "sharing.h"
#include "document/data/items/codeJson.h"
#include "document/data/items/commonJson.h"
#include "document/data/items/deviceJson.h"

using namespace std;
using namespace nlohmann;

namespace document
{
namespace files
{

DeviceStorage::DeviceStorage()
{
}

bool DeviceStorage::write(const data::ConfigData &c, int deviceId)
{
  bool ret = true;

  try {
    ret &= writeDeviceJson(c, deviceId);
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("export device: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  return ret;
}

bool DeviceStorage::import(Config &c)
{
  bool ret = true;

  try {
    ret &= importDeviceJson(c);
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("inport device: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  return ret;
}

bool DeviceStorage::importDeviceJson(Config &c)
{
  json j;

  auto file = QFileDialog::getOpenFileName(nullptr,
      tr("Import device from json"), QDir::homePath(),
      tr("json Files (*.json);;All Files (*)"));
  if (file.isEmpty()) {
    return false;
  }
  auto size = QFile(file).size();
  if (size == 0) {
    emit writeLog(LogLevel::Error, tr("load: %1 is empty").arg(file),
        ContentType::PlainText);
    return false;
  }

#ifdef _WIN32
    ifstream f(file.toStdWString().c_str(), ios::binary);
  #else
  ifstream f(file.toUtf8(), ios::binary);
#endif
  if (!f.is_open()) {
    emit writeLog(LogLevel::Error, tr("load: failed to open %1").arg(file),
        ContentType::PlainText);
    return false;
  }
  try {
    j = json::parse(f);
  } catch (const nlohmann::json::parse_error &e) {
    emit writeLog(LogLevel::Error,
        tr("load: Json parse error at byte %1: %2").arg(e.byte).arg(
            QString(e.what())), ContentType::PlainText);
    return false;
  } catch (const nlohmann::json::exception &e) {
    emit writeLog(LogLevel::Error,
        tr("load: Json error [%1]: %2").arg(e.id).arg(QString(e.what())),
        ContentType::PlainText);
    return false;
  }

  auto version = j["Version"].get<uint32_t>();
  if (version != jsonVersion) {
    emit writeLog(LogLevel::Error,
        tr("load %1: unsupported version %2").arg(file).arg(version),
        ContentType::PlainText);
    return false;

  }
  if (!j.contains("Device")) {
    emit writeLog(LogLevel::Error, tr("load %1: no Device entry").arg(file),
        ContentType::PlainText);
    QMessageBox msgBox(QMessageBox::Warning, tr("Import"),
        tr("This file conains no device to import (missing \"Device\" entry)."),
        QMessageBox::Ok);
    msgBox.exec();
    return false;
  }

  data::item::Device d(0); //will be overwritten anyway
  data::serialiser::fromJson(j["Device"], d);
  auto &worker = c.modify();

  //check if we have the required irProto lib entries to import this device
  for (const auto &cmd : d.getIrCommands().getProtoCommands()) {
    if (!cmd.codeType.get().isEnumValue()) {
      emit writeLog(LogLevel::Error,
          tr("load %1: unsupported protocol type %2").arg(file).arg(
              cmd.codeType.get().getQString()), ContentType::PlainText);
      QMessageBox msgBox(QMessageBox::Warning, tr("Import"),
          tr("Can't import this device, the protocol %1 is not available. "
              "Check for updates.").arg(cmd.codeType.get().getQString()),
          QMessageBox::Ok);
      msgBox.exec();
      return false;
    }
  }
  //...and now add them
  c.beginMacro(
      tr("Import Device %1").arg(QString::fromStdString(d.model.get())));

  for (auto &cmd : d.getIrCommands().getProtoCommands()) {
    auto protocolIndex = worker.appendIrProtoLibItem(cmd.codeType.get());
    if (protocolIndex < 0) {
      emit writeLog(LogLevel::Error,
          tr("load %1: adding protocol type %2 failed").arg(file).arg(
              cmd.codeType.get().getQString()), ContentType::PlainText);
      c.endMacro();
      return false;
    }
    cmd.protocolIndex = protocolIndex; //update protocol index
  }
  auto ret = worker.addDeviceCommand(d, -1);
  c.endMacro();
  return ret;
}

bool DeviceStorage::writeDeviceJson(const data::ConfigData &c, int deviceId)
{
  QString proprietary;
  QString deviceName;
  ordered_json j; //top level -> insertion order

  j["Version"] = jsonVersion;

  const auto *device = c.getDevice(deviceId);
  if (device == nullptr) {
    emit writeLog(LogLevel::Error,
        tr("inport device: invalid device id %1").arg(deviceId),
        ContentType::PlainText);
    return false;
  }
  //we reject exporting devices with proprietary commands:
  // - we don't own them -- they belong to Logitech
  // - we don't export the irproto lib part -> only "known" protocols will work at import!
  for (const auto &cmd : device->getIrCommands().getProtoCommands()) {
    switch (cmd.codeType.get().getValue()) {
      case data::CodeType::Proprietary:
      case data::CodeType::Unknown:
        proprietary = proprietary
            + QString::fromStdString(cmd.name.get() + "; ");
        break;
      default:
        break;
    }
  }
  if (!proprietary.isEmpty()) {
    proprietary.chop(2); // remove trailing "; "
    if (proprietary.length() > 80) {
      proprietary = proprietary.left(70) + "... (more)";
    }
    QMessageBox msgBox(QMessageBox::Warning, tr("Export"),
        tr("Can't export devices that contain proprietary IR commands from "
            "the Logitech Harmony software.\n\n"
            "You need to remove or re-learn the following commands: %1").arg(
            proprietary), QMessageBox::Ok);
    msgBox.exec();
    return false;
  }

  data::serialiser::toJson(j["Device"], *device);

  deviceName = QString::fromStdString(device->mnf.get()) + "-"
      + QString::fromStdString(device->model.get()) + ".json";

  auto file = QFileDialog::getSaveFileName(nullptr, tr("Save device as json"),
      QDir::homePath() + "/" + deviceName,
      tr("json Files (*.json);;All Files (*)"));
  if (file.isEmpty()) {
    return false;
  }

#ifdef _WIN32
  ofstream f(file.toStdWString().c_str(), ios::binary | ios::trunc);
#else
  ofstream f(file.toUtf8(), ios::binary | ios::trunc);
#endif
  if (!f.is_open()) {
    emit writeLog(LogLevel::Error, tr("write: failed to open %1").arg(file),
        ContentType::PlainText);
    return false;
  }
  try {
    f << j.dump(4);
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("load: Json serialise error: %1").arg(QString(e.what())),
        ContentType::PlainText);
    return false;
  }

  return true;
}

}
}
