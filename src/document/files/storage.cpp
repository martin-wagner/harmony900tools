// SPDX-License-Identifier: LGPL-2.1-or-later

#include <fstream>
#include <nlohmann/json.hpp>

#include "lib/uid.h"
#include "document/data/data.h"
#include "storage.h"
#include "document/data/items/activityJson.h"
#include "document/data/items/codeJson.h"
#include "document/data/items/commonJson.h"
#include "document/data/items/deviceJson.h"
#include "document/data/items/irProtoJson.h"

using namespace std;
using namespace nlohmann;

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
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("dump: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  return ret;
}

bool ConfigStorage::read(data::ConfigData &c)
{
  bool ret = true;

  try {
    ret &= readUserConfigJson(c);
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("load: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }


  return ret;
}

bool ConfigStorage::readUserConfigJson(data::ConfigData &c)
{
  int i;
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

  data::serialiser::fromJson(j["User"], c.getUser());
  data::serialiser::fromJson(j["Controller"], c.getController());
  for (i = 0; i < j["Devices"].size(); i++) {
    auto id = j["Devices"][i]["Id"].get<uint32_t>();
    lib::UidGenerator::getInstance().markUsed(id);
    data::item::Device d(id);
    data::serialiser::fromJson(j["Devices"][i], d);
    c.getDevices().push_back(d);
  }
  for (i = 0; i < j["Activities"].size(); i++) {
    auto id = j["Activities"][i]["Id"].get<uint32_t>();
    lib::UidGenerator::getInstance().markUsed(id);
    data::item::Activity a(id);
    data::serialiser::fromJson(j["Activities"][i], a);
    c.getActivities().push_back(a);
  }
  for (i = 0; i < j["Blobs"].size(); i++) {
    auto b = data::serialiser::fromJson(j["Blobs"][i]);
    c.getBlobs().push_back(b);
  }
  for (i = 0; i < j["Commands"].size(); i++) {
    binary::irProto::Code cmd;
    data::serialiser::fromJson(j["Commands"][i], cmd);
    c.getCommands().push_back(cmd);
  }
  for (i = 0; i < j["IrProtocols"].size(); i++) {
    binary::irProto::IrProto prot;
    data::serialiser::fromJson(j["IrProtocols"][i], prot);
    c.getProtocolLib().appendProtocol(prot);
  }

  return true;
}

bool ConfigStorage::writeUserConfigJson(const data::ConfigData &c)
{
  uint32_t i;
  ordered_json j; //top level -> insertion order

  j["Version"] = jsonVersion;

  data::serialiser::toJson(j["User"], c.getUser());
  data::serialiser::toJson(j["Controller"], c.getController());
  for (i = 0; i < c.getDevices().size(); i++) {
    data::serialiser::toJson(j["Devices"][i], c.getDevices()[i]);
  }
  for (i = 0; i < c.getActivities().size(); i++) {
    data::serialiser::toJson(j["Activities"][i], c.getActivities()[i]);
  }
  for (i = 0; i < c.getBlobs().size(); i++) {
    data::serialiser::toJson(j["Blobs"][i], c.getBlobs()[i]);
  }


  for (i = 0; i < c.getCommands().size(); i++) {
    data::serialiser::toJson(j["Commands"][i], c.getCommands()[i]);
  }
  for (i = 0; i < c.getProtocolLib().getProtocolCount(); i++) {
    data::serialiser::toJson(j["IrProtocols"][i], c.getProtocolLib().accessProtocol(i));
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

  return true;
}

}
}
