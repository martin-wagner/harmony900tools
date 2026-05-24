// SPDX-License-Identifier: LGPL-2.1-or-later

#include <pugixml.hpp>

#include "document/data/data.h"
#include "document/data/catalogue.h"
#include "h900.h"

using namespace std;

namespace document
{
namespace files
{

ConfigH900::ConfigH900(const QString &workPath) : wp(workPath)
{
}

bool ConfigH900::dump(const data::ConfigData &c)
{
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

  } catch (...) {
    emit writeLog(LogLevel::Error, tr("import: failed with exception"),
        ContentType::PlainText);
    return false;
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
    auto ret = worker->addDeviceCommand(id);
    if (!ret) {
      continue;
    }

    //todo all the other stuff...

  }

  //activities //todo


  //protocols //todo


  return true;
}

}
}
