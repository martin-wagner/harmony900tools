// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <pugixml.hpp>

#include "document/data/data.h"
#include "document/data/catalogue.h"
#include "h900.h"

using namespace std;

namespace document
{
namespace files
{

ConfigH900::ConfigH900(const QString &workPath) :
    wp(workPath)
{
}

bool ConfigH900::dump(const data::ConfigData &c)
{
  bool ret = true;

  try {
    ret &= dumpUserConfigXml(c);

//  pugi::xml_document actionList;
//  pugi::xml_document userConfiguration;
//  std::vector<uint8_t> irProto;
//  std::vector<uint8_t> ssir;

  } catch (const std::exception &e) {
    emit writeLog(LogLevel::Error,
        tr("export: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  return ret;
}

bool ConfigH900::read(data::ConfigData &c, data::CmdCatalogue *worker)
{
  bool ret = true;

  try {
    ret &= readUserConfigXml(c, worker);

//  pugi::xml_document actionList;
//  pugi::xml_document userConfiguration;
//  std::vector<uint8_t> irProto;
//  std::vector<uint8_t> ssir;

  } catch (const std::exception &e) {
    emit writeLog(LogLevel::Error,
        tr("import: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
  }

  return ret;
}

bool ConfigH900::readUserConfigXml(data::ConfigData &c,
    data::CmdCatalogue *worker)
{
  pugi::xml_parse_result ret;
  pugi::xml_document xml;

#ifdef _WIN32
  ret = xml.load_file(QString(wp + "/" + userConfigPath).toStdWString().c_str());
#else
  ret = xml.load_file(QString(wp + "/" + userConfigPath).toUtf8());
#endif
  if (!ret) {
    emit writeLog(LogLevel::Error,
        tr("import: failed to open %1 (%2 at %3)").arg(
            wp + "/" + userConfigPath).arg(QString(ret.description())).arg(
            ret.offset), ContentType::PlainText);
    return false;
  }

  auto root = xml.child("Root");

  //general stuff
  for (pugi::xml_node prop : root.child("Properties").children("Property")) {
    if (std::string(prop.attribute("name").as_string()) == "version") {
      QString version = prop.child_value();
      if (version != "1.0") {
        emit writeLog(LogLevel::Error,
            tr("import %1: unsupported version %2").arg(userConfigPath).arg(
                version), ContentType::PlainText);
        return false;
      }
    }
    if (std::string(prop.attribute("name").as_string()) == "LastUpdated") {
      QString upd = prop.child_value();
      emit writeLog(LogLevel::Info,
          tr("import %1: last update at %2").arg(userConfigPath).arg(upd),
          ContentType::PlainText);
    }
  }

  //user //todo

  //controller //todo

  //devices
  for (pugi::xml_node device : root.children("Device")) {
    auto id = device.child("Id").text().as_uint();
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

bool ConfigH900::dumpUserConfigXml(const data::ConfigData &c)
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
  for (const auto &d : c.getDevices()) {
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
