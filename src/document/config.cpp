// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>

#include "lib/zip.h"
#include "lib/uid.h"
#include "config.h"
#include "files/h900.h"
#include "files/storage.h"

using namespace std;

namespace document
{

Config::Config(Context &ctx, bool init) :
    stack(ctx.getUndoStack())
{
  reset();

  if (init) {
    create();
  }
}

bool document::Config::create()
{
  reset();

  //todo
}

bool Config::read(const std::vector<uint8_t> &zip, Type t)
{
  bool ret = false;
  QTemporaryFile zipFile;

  reset();

  if (t != Type::H900) {
    emit writeLog(LogLevel::Error, tr("Type not supported (%1)").arg((int) t),
        ContentType::PlainText);
    return false;
  }
  type = t;

  if (workPath.isEmpty()) {
    return false;
  }

  // write the zip buffer to a temp file so minizip can open it by path
  zipFile.setAutoRemove(true);
  if (!zipFile.open()) {
    emit writeLog(LogLevel::Error, tr("config: failed to create temp zip file"),
        ContentType::PlainText);
    return false;
  }
  emit writeLog(LogLevel::Debug, tr("config: using temp file %1, "
      "temp dir %2").arg(zipFile.fileName()).arg(workPath),
      ContentType::PlainText);

  zipFile.write(reinterpret_cast<const char*>(zip.data()),
      static_cast<qint64>(zip.size()));
  zipFile.flush();
  zipFile.close();

  auto zipPath = QFile::encodeName(zipFile.fileName());
#ifdef _WIN32
  auto uf = unzOpen64(zipFile.fileName().toStdWString().c_str());
#else
  auto uf = unzOpen64(QFile::encodeName(zipFile.fileName()).constData());
#endif
  if (uf == nullptr) {
    emit writeLog(LogLevel::Error, tr("config: failed to open temp zip file"),
        ContentType::PlainText);
    return false;
  }

  auto ok = lib::unzipToDirectory(uf, workPath);
  unzClose(uf);

  if (!ok) {
    emit writeLog(LogLevel::Error, tr("config: extraction failed"),
        ContentType::PlainText);
    return false;
  }

  // TODO: config.configData.load(config.workPath);
  //todo deserialise
  //todo setup uids

  switch (t) {
    case Type::H900: {
      auto parser = files::ConfigH900(workPath);
      connect(&parser, &files::ConfigH900::writeLog, this, &Config::writeLog);
      connect(&parser, &files::ConfigH900::writeMsg, this, &Config::writeMsg);
      stack.beginMacro(tr("Import Harmony 900 Config"));
      ret = parser.read(*configData, worker);
      stack.endMacro();
      break;
    }
    default:
      //must not happen, this is checked first
      return false;
  }

  dirty = true;
  return ret;
}

bool Config::read(const QString &path)
{
  reset();

  if (path.isEmpty()) {
    emit writeMsg(tr("No path set"));
    return false;
  }
  savePath = path;

  auto storage = files::ConfigStorage(savePath);
  connect(&storage, &files::ConfigStorage::writeLog, this, &Config::writeLog);
  connect(&storage, &files::ConfigStorage::writeMsg, this, &Config::writeMsg);
  stack.beginMacro(tr("Read Config"));
  auto ret = storage.read(*configData, worker);
  stack.endMacro();
  dirty = true;
  return ret;
}

bool Config::reset()
{
  if (worker != nullptr) {
    disconnect(worker, &data::CmdCatalogue::writeLog, this, &Config::writeLog);
    disconnect(worker, &data::CmdCatalogue::writeMsg, this, &Config::writeMsg);
  }

  configData = make_unique<data::ConfigData>();
  worker = new data::CmdCatalogue(*configData, stack, this);
  type = Type::UNKNOWN;
  lib::UidGenerator::initialize(UidStartValue);
  workPath.clear();
  savePath.clear();
  dirty = false;

  tempDir = std::make_unique<QTemporaryDir>();
  if (!tempDir->isValid()) {
    qWarning() << "Config::create(zip): failed to create temp dir";
    return false;
  }
  tempDir->setAutoRemove(true);
  workPath = tempDir->path();

  connect(worker, &data::CmdCatalogue::writeLog, this, &Config::writeLog);
  connect(worker, &data::CmdCatalogue::writeMsg, this, &Config::writeMsg);

  return true;
}

Config::~Config()
{
}

bool Config::isDirty()
{
}

QString Config::getPath()
{
}

bool Config::save()
{
  if (savePath.isEmpty()) {
    emit writeMsg(tr("No path set"));
    return false;
  }
  return saveAs(savePath);
}

bool Config::saveAs(const QString &path)
{
  if (path.isEmpty()) {
    emit writeMsg(tr("Path is empty"));
    return false;
  }
  savePath = path;

  auto storage = files::ConfigStorage(savePath);
  connect(&storage, &files::ConfigStorage::writeLog, this, &Config::writeLog);
  connect(&storage, &files::ConfigStorage::writeMsg, this, &Config::writeMsg);
  auto ret = storage.write(*configData);
  if (ret == true) {
    dirty = false;
  }
  return ret;
}

bool Config::dumpZip(std::vector<uint8_t> &zip, Type t)
{
  QTemporaryFile zipFile;

  if (t != Type::H900) {
    emit writeLog(LogLevel::Error, tr("Type not supported (%1)").arg((int) t),
        ContentType::PlainText);
    return false;
  }
  type = t;

  // TODO: save configData to workPath first
  //todo serialise

  zipFile.setAutoRemove(true);
  if (!zipFile.open()) {
    emit writeLog(LogLevel::Error, tr("config: failed to create temp zip file"),
        ContentType::PlainText);
    return false;
  }
  auto zipFilePath = zipFile.fileName();
  zipFile.close(); // hand off to minizip; file stays on disk until zipFile is destroyed

  auto zipPath = QFile::encodeName(zipFilePath);
#ifdef _WIN32
  auto zf = zipOpen64(zipFilePath.toStdWString().c_str(), APPEND_STATUS_CREATE);
#else
  auto zf = zipOpen64(QFile::encodeName(zipFilePath).constData(),
  APPEND_STATUS_CREATE);
#endif
  if (zf == nullptr) {
    emit writeLog(LogLevel::Error, tr("config: failed to open temp zip file"),
        ContentType::PlainText);
    return false;
  }

  auto ok = lib::zipDirectory(zf, workPath);
  if (!ok) {
    zipClose(zf, nullptr);
    return false;
  }
  zipClose(zf, nullptr);

  QFile result(zipFilePath);
  if (!result.open(QIODevice::ReadOnly)) {
    emit writeLog(LogLevel::Error, tr("config: compression failed"),
        ContentType::PlainText);
    return false;
  }
  QByteArray data = result.readAll();
  zip.assign(data.constBegin(), data.constEnd());

  return true;
}

}
