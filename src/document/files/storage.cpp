// SPDX-License-Identifier: LGPL-2.1-or-later

#include <fstream>
#include <nlohmann/json.hpp>

#include "document/data/data.h"
#include "document/data/catalogue.h"
#include "storage.h"

using namespace std;
using json = nlohmann::json;

namespace document
{
namespace files
{

ConfigStorage::ConfigStorage(const QString &workPath) :
    wp(workPath)
{
}

bool ConfigStorage::write(const data::ConfigData &c)
{
  bool ret = true;

  try {
    ret &= writeUserConfigJson(c);

    //todo binary data
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("dump: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  return ret;
}

bool ConfigStorage::read(data::ConfigData &c, data::CmdCatalogue *worker)
{
  bool ret = true;

  try {
    ret &= readUserConfigJson(c, worker);

//  pugi::xml_document actionList;
//  pugi::xml_document userConfiguration;
//  vector<uint8_t> irProto;
//  vector<uint8_t> ssir;

  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("load: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  return ret;
}

bool ConfigStorage::readUserConfigJson(data::ConfigData &c,
    data::CmdCatalogue *worker)
{
  json j;

#ifdef _WIN32
  ifstream f(QString(wp + "/" + jsonPath).toStdWString().c_str(), ios::binary);
#else
  ifstream f(QString(wp + "/" + jsonPath).toUtf8(), ios::binary);
#endif
  if (!f.is_open()) {
    emit writeLog(LogLevel::Error,
        tr("load: failed to open %1").arg(wp + "/" + jsonPath),
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
        tr("load %1: unsupported version %2").arg(wp).arg(version),
        ContentType::PlainText);
    return false;

  }

  //general stuff todo
  //user //todo
  //controller //todo

  //devices
  for (int i = 0; i < j["Devices"].size(); i++) {
    auto &jd = j["Devices"][i];

    auto id = jd["Id"].get<uint32_t>();
    auto ret = worker->addDeviceCommand(-1, id);
    if (!ret) {
      continue;
    }

    //todo all the other stuff...
  }



  //activities //todo

  //protocols //todo

  return true;
}

bool ConfigStorage::writeUserConfigJson(const data::ConfigData &c)
{
  bool ret = true;
  json j;

  j["Version"] = jsonVersion;
  j["User"]["FirstName"] = c.getUser().getFirstName();
  j["User"]["LastName"] = c.getUser().getLastName();
  j["User"]["Login"] = c.getUser().getOsUserName();
  j["User"]["Created"] = c.getUser().getFileCreationDate();
  j["User"]["Modified"] = c.getUser().getFileModificationDate();

  for (int i = 0; i < c.getDevices().size(); i++) {
    auto &jd = j["Devices"][i];
    auto &d = c.getDevices()[i];
    jd["Id"] = d.getId();
    jd["Test"] = 42;

  }

#ifdef _WIN32
  ofstream f(QString(wp + "/" + jsonPath).toStdWString().c_str(), ios::binary | ios::trunc);
#else
  ofstream f(QString(wp + "/" + jsonPath).toUtf8(), ios::binary | ios::trunc);
#endif
  if (!f.is_open()) {
    emit writeLog(LogLevel::Error,
        tr("write: failed to open %1").arg(wp + "/" + jsonPath),
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

  return ret;
}

}
}
