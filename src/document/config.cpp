// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>

#include "lib/zip.h"
#include "lib/uid.h"
#include "config.h"
#include "files/h900.h"
#include "files/create.h"
#include "files/storage.h"

using namespace std;

namespace document
{

Config::Config(Context &ctx, bool init, QObject *parent) :
    QObject(parent), stack(ctx.undoStack())
{
  reset();

  if (init) {
    create();
  }
}

bool document::Config::create()
{
  stack.beginMacro(tr("Create new Harmony 900 Config"));

  reset();

  auto creator = files::Create(*(configData.get()));
  connect(&creator, &files::Create::writeLog, this, &Config::writeLog);
  connect(&creator, &files::Create::writeMsg, this, &Config::writeMsg);
  creator.write(worker);
  stack.endMacro();
  return true;
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

  if (importPath.isEmpty()) {
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
      "temp dir %2").arg(zipFile.fileName()).arg(importPath),
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

  auto ok = lib::unzipToDirectory(uf, importPath);
  unzClose(uf);
  if (!ok) {
    emit writeLog(LogLevel::Error, tr("config: extraction failed"),
        ContentType::PlainText);
    return false;
  }

  // TODO: config.configData.load(config.importPath);
  //todo deserialise
  //todo setup uids
  //todo backup zip to project file

  switch (t) {
    case Type::H900: {
      auto parser = files::ConfigH900(importPath);
      connect(&parser, &files::ConfigH900::writeLog, this, &Config::writeLog);
      connect(&parser, &files::ConfigH900::writeMsg, this, &Config::writeMsg);
      stack.beginMacro(tr("Import Harmony 900 Config")); //macro -> speedup
      ret = parser.read(configData.get(), worker);
      stack.endMacro();
      stack.clear(); // no undo for this!
      break;
    }
    default:
      //must not happen, this is checked first
      return false;
  }

  dirty = true;
  return ret;
}

bool Config::read(const QString &file)
{
  bool ret;

  reset();

  if (file.isEmpty()) {
    emit writeMsg(tr("No path set"));
    return false;
  }

#ifdef _WIN32
  auto uf = unzOpen64(file.toStdWString().c_str());
#else
  auto uf = unzOpen64(QFile::encodeName(file).constData());
#endif
  if (uf == nullptr) {
    emit writeLog(LogLevel::Error,
        tr("config: failed to open compressed config"), ContentType::PlainText);
    return false;
  }

  auto ok = lib::unzipToDirectory(uf, tempDir->path());
  unzClose(uf);
  if (!ok) {
    emit writeLog(LogLevel::Error, tr("config: extraction failed"),
        ContentType::PlainText);
    return false;
  }

  auto storage = files::ConfigStorage(workPath);
  connect(&storage, &files::ConfigStorage::writeLog, this, &Config::writeLog);
  connect(&storage, &files::ConfigStorage::writeMsg, this, &Config::writeMsg);
  ret = storage.read(*configData);
  stack.clear(); // no undo for this!
  emit dirtyChanged(true);
  return ret;
}

bool Config::reset()
{
  QDir dir;

  if (worker != nullptr) {
    worker->deleteLater();
  }

  configData = make_unique<data::ConfigData>();
  worker = new data::CmdCatalogue(*configData, stack, this);
  lib::UidGenerator::initialize(UidStartValue);
  workPath.clear();
  importPath.clear();
  dirty = false;

  tempDir = std::make_unique<QTemporaryDir>();
  if (!tempDir->isValid()) {
    qWarning() << "Config::create(zip): failed to create temp dir";
    return false;
  }
  tempDir->setAutoRemove(true);
  dir.mkpath(tempDir->path());
  importPath = tempDir->path() + "/import";
  exportPath = tempDir->path() + "/export";
  workPath = tempDir->path() + "/project";
  dir.mkpath(importPath);
  dir.mkpath(exportPath);
  dir.mkpath(workPath);

  // @formatter:off
  connect(worker, &data::CmdCatalogue::writeLog, this, &Config::writeLog);
  connect(worker, &data::CmdCatalogue::writeMsg, this, &Config::writeMsg);
  connect(worker, &data::CmdCatalogue::itemChanged, this, &Config::itemChanged);
  connect(worker, &data::CmdCatalogue::itemAboutToBeAdded, this, &Config::itemAboutToBeAdded);
  connect(worker, &data::CmdCatalogue::itemAdded, this, &Config::itemAdded);
  connect(worker, &data::CmdCatalogue::itemAboutToBeRemoved, this, &Config::itemAboutToBeRemoved);
  connect(worker, &data::CmdCatalogue::itemRemoved, this, &Config::itemRemoved);
  connect(worker, &data::CmdCatalogue::dirtyChanged, this, [this](bool dirty) {
    this->dirty = dirty;
    emit dirtyChanged(dirty);
  });
// @formatter:on

  return true;
}

Config::~Config()
{
}

bool Config::isDirty()
{
  return dirty;
}

bool Config::saveAs(const QString &file)
{
  if (file.isEmpty()) {
    emit writeMsg(tr("Path is empty"));
    return false;
  }

  auto storage = files::ConfigStorage(workPath);
  connect(&storage, &files::ConfigStorage::writeLog, this, &Config::writeLog);
  connect(&storage, &files::ConfigStorage::writeMsg, this, &Config::writeMsg);
  configData->getUser().fileModificationDate.set(lib::writeTime()); //update save date, no undo stack!
  auto ret = storage.write(*configData);
  if (ret == true) {
    dirty = false;
  } else {
    return false;
  }

#ifdef _WIN32
  auto zf = zipOpen64(file.toStdWString().c_str(), APPEND_STATUS_CREATE);
#else
  auto zf = zipOpen64(QFile::encodeName(file).constData(),
  APPEND_STATUS_CREATE);
#endif
  if (zf == nullptr) {
    emit writeLog(LogLevel::Error, tr("config: failed to create project file"),
        ContentType::PlainText);
    return false;
  }

  auto ok = lib::zipDirectory(zf, tempDir->path());
  if (!ok) {
    zipClose(zf, nullptr);
    return false;
  }
  zipClose(zf, nullptr);
  return true;
}

bool Config::dumpZip(std::vector<uint8_t> &zip, Type t) const
{
  bool ret;
  QTemporaryFile zipFile;

  switch (t) {
    case Type::H900: {
      auto parser = files::ConfigH900(exportPath);
      connect(&parser, &files::ConfigH900::writeLog, this, &Config::writeLog);
      connect(&parser, &files::ConfigH900::writeMsg, this, &Config::writeMsg);
      ret = parser.dump(configData.get());
      break;
    }
    default:
      emit writeLog(LogLevel::Error, tr("Type not supported (%1)").arg((int) t),
          ContentType::PlainText);
      return false;
  }
  if (!ret) {
    return ret;
  }

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

  auto ok = lib::zipDirectory(zf, exportPath, {".postinstall", ".preinstall"});
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

const data::ConfigData& Config::data() const
{
  return *configData;
}

data::CmdCatalogue& Config::modify()
{
  return *worker;
}

void Config::beginMacro(const QString &text)
{
  stack.beginMacro(text);
}

void Config::endMacro()
{
  stack.endMacro();
}

}
