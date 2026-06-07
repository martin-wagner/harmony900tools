// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <pugixml.hpp>

#include "lib/consthash.h"
#include "lib/uid.h"
#include "document/data/data.h"
#include "document/data/catalogue.h"
#include "document/data/items/unknown.h"
#include "h900.h"

using namespace std;
using namespace document::data;

namespace document
{
namespace files
{

ConfigH900::ConfigH900(const QString &workPath) :
    wp(workPath)
{
}

bool ConfigH900::dump(const ConfigData *c)
{
  bool ret = true;

  this->c = c;
  this->worker = nullptr;

  try {
    ret &= dumpUserConfigXml();

//  pugi::xml_document actionList;
//  pugi::xml_document userConfiguration;
//  vector<uint8_t> irProto;
//  vector<uint8_t> ssir;

  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("export: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  this->c = nullptr;

  return ret;
}

bool ConfigH900::read(const ConfigData *c, CmdCatalogue *worker)
{
  bool ret = true;

  this->c = c;
  this->worker = worker;

  try {
    ret &= readUserConfigXml();

//  pugi::xml_document actionList;
//  pugi::xml_document userConfiguration;
//  vector<uint8_t> irProto;
//  vector<uint8_t> ssir;

  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("import: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  this->c = nullptr;
  this->worker = nullptr;

  return ret;
}

bool ConfigH900::readUserConfigXml()
{
  pugi::xml_parse_result res;
  pugi::xml_document xml;

#ifdef _WIN32
  res = xml.load_file(QString(wp + "/" + userConfigPath).toStdWString().c_str());
#else
  res = xml.load_file(QString(wp + "/" + userConfigPath).toUtf8());
#endif
  if (!res) {
    emit writeLog(LogLevel::Error,
        tr("import: failed to open %1 (%2 at %3)").arg(
            wp + "/" + userConfigPath).arg(QString(res.description())).arg(
            res.offset), ContentType::PlainText);
    return false;
  }

  emit writeLog(LogLevel::Debug, tr("Importing xml"), ContentType::PlainText);

  auto root = xml.child("Root");
  auto ret = readProperties(root); //version check
  if (!ret) {
    return ret;
  }
  ret &= readUser(root);
  ret &= readController(root);
  ret &= readDevices(root);
  ret &= readActivities(root);
  ret &= readProtocols(root);
  return ret;
}

bool ConfigH900::readProperties(pugi::xml_node &root)
{
  QString s;

  for (pugi::xml_node prop : root.child("Properties").children("Property")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "version"_hash: {
        QString version = prop.child_value();
        if (version != "1.0") {
          emit writeLog(LogLevel::Error,
              tr("import %1: unsupported version %2").arg(userConfigPath).arg(
                  version), ContentType::PlainText);
          return false;
        }
        break;
      }
      case "ProtocolCacheHash"_hash: {
        QString s = prop.child_value();
        emit writeLog(LogLevel::Debug,
            tr("import %1: irproto crc %2").arg(userConfigPath).arg(s),
            ContentType::PlainText);
        break;
      }
      case "LastUpdated"_hash: {
        QString s = prop.child_value();
        emit writeLog(LogLevel::Info,
            tr("import %1: last update at %2").arg(userConfigPath).arg(s),
            ContentType::PlainText);
        break;
      }
    }
  }
  return true;
}

bool ConfigH900::readUser(pugi::xml_node &root)
{
  unordered_set<string> properties;

  Property<bool> newDeviceFound { false, Include::CHECK };
  Property<bool> trainingWheels { false, Include::ALWAYS };
  Property<Enum<Locale>> locale { { Locale::enu }, Include::ALWAYS };
  Property<Enum<TimeFormat>> timeFormat { { TimeFormat::Military },
      Include::ALWAYS };

  auto user = root.child("User");

  auto id = user.child("Id").text().as_uint();
  addId(id);
  worker->setUserId(id);
  auto firstName = user.child("Presentation").child("FirstName").child_value();
  worker->setUserName(QString::fromStdString(firstName),
      QString::fromStdString(lastName));
  auto lastName = user.child("Presentation").child("LastName").child_value();
  worker->setUserMetadata();

  for (pugi::xml_node prop : user.child("Properties").children("Property")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "TrainingWheels"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setUserTrainingWheels(val);
        break;
      }
      case "NewDeviceFound"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setUserNewDeviceFound(val);
        break;
      }
      case "LocaleId"_hash: {
        string str = prop.child_value();
        auto loc = Enum<Locale>(str);
        worker->setUserLocale(loc);
        break;
      }
      case "TimeDisplayFormat"_hash: {
        string str = prop.child_value();
        auto t = Enum<TimeFormat>(str);
        worker->setUserTimeFormat(t);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);

      }
    }
  }
  return true;
}

bool ConfigH900::readController(pugi::xml_node &root)
{
  return true;
}

bool ConfigH900::readDevices(pugi::xml_node &root)
{

  //devices
  for (pugi::xml_node device : root.children("Device")) {
    auto id = device.child("Id").text().as_uint();
    auto ret = worker->addDeviceCommand(-1, id);
    if (!ret) {
      continue;
    }

    //todo all the other stuff...

  }

  return true;
}

bool ConfigH900::readDevice(pugi::xml_node &devices)
{
  return true;
}

bool ConfigH900::readActivities(pugi::xml_node &root)
{
  return true;
}

bool ConfigH900::readActivitiy(pugi::xml_node &activities)
{
  return true;
}

bool ConfigH900::readProtocols(pugi::xml_node &root)
{
  return true;
}

bool ConfigH900::readProtocol(pugi::xml_node &protocols)
{
  return true;
}

data::item::UnknownElement ConfigH900::toUnknownElement(
    const pugi::xml_node &node)
{
  std::map<std::string, std::string> attrs;
  std::vector<data::item::UnknownElement> children;

  for (pugi::xml_attribute attr : node.attributes()) {
    attrs.emplace(attr.name(), attr.as_string());
  }

  for (pugi::xml_node child : node.children()) {
    // skip pure text nodes if you want clean reconstruction
    if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata)
      continue;

    children.emplace_back(toUnknownElement(child));
  }

  std::string text;

  // only take meaningful text (trim-ish behavior optional)
  if (node.first_child() && node.first_child().type() == pugi::node_pcdata) {
    text = node.child_value();
  }

  return data::item::UnknownElement(node.name() ? node.name() : "", attrs, text,
      children);
}

void ConfigH900::addId(uint32_t id)
{
  lib::UidGenerator::getInstance().markUsed(id);
}

bool ConfigH900::dumpUserConfigXml()
{
  pugi::xml_document xml;
  bool ret;

  auto decl = xml.prepend_child(pugi::node_declaration);
  decl.append_attribute("version").set_value("1.0");
  decl.append_attribute("encoding").set_value("UTF-8");

  auto root = xml.append_child("Root");

  //general stuff
  auto properties = root.append_child("Properties");
  auto property = properties.append_child("Property");
  property.append_attribute("name").set_value("version");
  property.text().set("1.0");

  //user //todo

  //controller //todo

  //devices
  for (const auto &d : c->getDevices()) {
    auto devices = root.append_child("Device");
    devices.append_child("Id").text().set(d.getId());

    //todo all the other stuff...

  }

  //activities //todo

  //protocols //todo

  QDir().mkpath(wp + "/" + QFileInfo(userConfigPath).path());
#ifdef _WIN32
  ret = xml.save_file(QString(wp + "/" + userConfigPath).toStdWString().c_str(),
      PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
#else
  ret = xml.save_file(QString(wp + "/" + userConfigPath).toUtf8(),
      PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
#endif
  return ret;
}

}
}
