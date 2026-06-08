// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <pugixml.hpp>

#include "lib/consthash.h"
#include "lib/uid.h"
#include "lib/timestamp.h"
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

//not a class member!. for generic types
template<typename T>
void writeProperty(pugi::xml_node &parent, const char *name,
    const Property<T> &prop)
{
  if (prop.isIncluded() != Include::ALWAYS) {
    return;
  }
  auto property = parent.append_child("Property");
  property.append_attribute("name").set_value(name);
  property.text().set(prop.get());
}

//not a class member!. for enum types
template<typename T>
void writeProperty(pugi::xml_node &parent, const char *name,
    const Property<Enum<T>> &prop)
{
  if (prop.isIncluded() != Include::ALWAYS) {
    return;
  }
  auto property = parent.append_child("Property");
  property.append_attribute("name").set_value(name);
  property.text().set(prop.get().getString().c_str());
}

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
  auto user = root.child("User");

  auto id = user.child("Id").text().as_uint();
  addId(id);
  worker->setUserId(id);
  auto firstName = user.child("Presentation").child("FirstName").child_value();
  auto lastName = user.child("Presentation").child("LastName").child_value();
  worker->setUserName(QString::fromStdString(firstName),
      QString::fromStdString(lastName));
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
        emit writeLog(LogLevel::Debug,
            tr("import user: unknown property (value = %1)").arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
        worker->setUserUnknownProperty(unknown);
        break;
      }
    }
  }
  return true;
}

bool ConfigH900::readController(pugi::xml_node &root)
{
  auto controller = root.child("Controller");

  auto id = controller.child("Id").text().as_uint();
  addId(id);
  worker->setControllerId(id);

  auto type = controller.child("Type").child_value();
  auto mnf = controller.child("Manufacturer").child_value();
  auto model = controller.child("Model").child_value();
  auto label = controller.child("Presentation").child("Label").child_value();
  worker->setControllerMetadata(type, mnf, model, label);

  for (pugi::xml_node prop : controller.child("Properties").children("Property")) {
    //no properties by default
    auto unknown = toUnknownElement(prop);
    emit writeLog(LogLevel::Debug,
        tr("import user: unknown property (value = %1)").arg(
            QString::fromStdString(unknown.text)), ContentType::PlainText);
    worker->setControllerUnknownProperty(unknown);
  }
  return true;
}

bool ConfigH900::readDevices(pugi::xml_node &root)
{
  bool ret = true;

  for (pugi::xml_node device : root.children("Device")) {
    ret &= readDevice(device);
  }
  return ret;
}

bool ConfigH900::readDevice(pugi::xml_node &device)
{
  auto id = device.child("Id").text().as_uint();
  addId(id);
  auto ret = worker->addDeviceCommand(-1, id); //append
  if (!ret) {
    return false;
  }
  auto pos = c->getDevices().size() - 1;

  auto typeStr = device.child("Type").child_value();
  Enum<DeviceType> type(typeStr);
  auto mnf = device.child("Manufacturer").child_value();
  auto model = device.child("Model").child_value();
  auto label = device.child("Presentation").child("Label").child_value();
  worker->setDeviceMetadata(type, mnf, model, label, pos);

  for (pugi::xml_node prop : device.child("Properties").children("Property")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "AlwaysOn"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceAlwaysOn(val, pos);
        break;
      }
      case "AudioSwitch"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceAudioSwitch(val, pos);
        break;
      }
      case "AutoPower"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceAutoPower(val, pos);
        break;
      }
      case "Dimmer"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceDimmer(val, pos);
        break;
      }
      case "HasBands"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceHasBands(val, pos);
        break;
      }
      case "HasPresets"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceHasPresets(val, pos);
        break;
      }
      case "IsDisplayDevice"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceIsDisplayDevice(val, pos);
        break;
      }
      case "IsNewDevice"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceIsNewDevice(val, pos);
        break;
      }
      case "ManualPower"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceManualPower(val, pos);
        break;
      }
      case "MenuOnDevice"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceMenuOnDevice(val, pos);
        break;
      }
      case "NumDiscs"_hash: {
        auto val = prop.text().as_int(1);
        worker->setDeviceNumDiscs(val, pos);
        break;
      }
      case "NumLights"_hash: {
        auto val = prop.text().as_int(1);
        worker->setDeviceNumLights(val, pos);
        break;
      }
      case "OnScreenGuide"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceOnScreenGuide(val, pos);
        break;
      }
      case "PvrType"_hash: {
        auto valStr = prop.text().as_string("Generic");
        auto val = Enum<PvrType>(valStr);
        worker->setDevicePvrType(val, pos);
        break;
      }
      case "RecordMedia Fixed Disc"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceRecordMediaFixedDisc(val, pos);
        break;
      }
      case "RecordMedia Removable Videotape"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceRecordMediaRemovableVideotape(val, pos);
        break;
      }
      case "RevertInput"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceRevertInput(val, pos);
        break;
      }
      case "Scart"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceScart(val, pos);
        break;
      }
      case "TunerInput"_hash: {
        auto valStr = prop.text().as_string("Tuner");
        auto val = Enum<TunerInput>(valStr);
        worker->setDeviceTunerInput(val, pos);
        break;
      }
      case "VideoSwitch"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setDeviceVideoSwitch(val, pos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import device: unknown property (value = %1)").arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
        worker->setDeviceUnknownProperty(unknown, pos);
        break;
      }
    }
  }

  //todo weitere...

  return true;
}

bool ConfigH900::readActivities(pugi::xml_node &root)
{
  return true;
}

bool ConfigH900::readActivitiy(pugi::xml_node &activitie)
{
  return true;
}

bool ConfigH900::readProtocols(pugi::xml_node &root)
{
  return true;
}

bool ConfigH900::readProtocol(pugi::xml_node &protocol)
{
  return true;
}

data::item::UnknownElement ConfigH900::toUnknownElement(
    const pugi::xml_node &node)
{
  string text;
  map<string, string> attrs;
  vector<data::item::UnknownElement> children;

  for (pugi::xml_attribute attr : node.attributes()) {
    attrs.emplace(attr.name(), attr.as_string());
  }

  for (pugi::xml_node child : node.children()) {
    // skip pure text nodes if you want clean reconstruction
    if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata)
      continue;

    children.emplace_back(toUnknownElement(child));
  }

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

  auto decl = xml.prepend_child(pugi::node_declaration);
  decl.append_attribute("version").set_value("1.0");
  decl.append_attribute("encoding").set_value("UTF-8");

  auto root = xml.append_child("Root");
  auto ret = writeProperties(root);
  ret &= writeUser(root);
  ret &= writeController(root);
  ret &= writeDevices(root);
  ret &= writeActivities(root);
  ret &= writeProtocols(root);
  if (!ret) {
    return ret;
  }

  QDir().mkpath(wp + "/" + QFileInfo(userConfigPath).path());
#ifdef _WIN32
  ret = xml.save_file(QString(wp + "/" + userConfigPath).toStdWString().c_str(),
      PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
#else
  ret = xml.save_file(QString(wp + "/" + userConfigPath).toUtf8(),
      PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
#endif
  return true;
}

bool ConfigH900::writeProperties(pugi::xml_node &root)
{
  auto properties = root.append_child("Properties");
  auto property = properties.append_child("Property");
  property.append_attribute("name").set_value("version");
  property.text().set("1.0");
  property = properties.append_child("Property");
  property.append_attribute("name").set_value("ProtocolCacheHash");
  property.text().set("0xdeadbeef"); //todo get the crc32
  property = properties.append_child("Property");
  property.append_attribute("name").set_value("LastUpdated");
  auto time = lib::writeTimeH900Xml();
  property.text().set(time);
  return true;
}

bool ConfigH900::writeUser(pugi::xml_node &root)
{
  auto user = root.append_child("User");
  auto id = user.append_child("Id");
  id.text().set(c->getUser().getId());
  // @formatter:off
  auto properties = user.append_child("Properties");
  writeProperty(properties, "TrainingWheels", c->getUser().trainingWheels);
  writeProperty(properties, "NewDeviceFound", c->getUser().newDeviceFound);
  writeProperty(properties, "LocaleId", c->getUser().locale);
  writeProperty(properties, "TimeDisplayFormat", c->getUser().timeFormat);
  for (const auto &prop : c->getUser().getUnknownProperties()) {
    writeUnknownElement(properties, prop);
  }
  auto presentation = user.append_child("Presentation");
  presentation.append_child("FirstName").text().set(c->getUser().firstName.get());
  presentation.append_child("LastName").text().set(c->getUser().lastName.get());
// @formatter:on
  return true;
}

bool ConfigH900::writeController(pugi::xml_node &root)
{
  // @formatter:off
  auto controller = root.append_child("Controller");
  controller.append_child("Id").text().set(c->getController().getId());
  controller.append_child("Type").text().set(c->getController().type.get());
  controller.append_child("Manufacturer").text().set(c->getController().mnf.get());
  controller.append_child("Model").text().set(c->getController().model.get());
  auto properties = controller.append_child("Properties");
  for (const auto &prop : c->getUser().getUnknownProperties()) {
    writeUnknownElement(properties, prop);
  }
  auto presentation = controller.append_child("Presentation");
  presentation.append_child("Label").text().set(c->getController().label.get());
// @formatter:on
  return true;
}

bool ConfigH900::writeDevices(pugi::xml_node &root)
{
  bool ret = true;

  const auto &devices = c->getDevices();
  for (uint32_t i = 0; i < devices.size(); i++) {
    auto device = root.append_child("Device");
    ret &= writeDevice(device, devices[i], i);
  }
  return ret;
}

bool ConfigH900::writeDevice(pugi::xml_node &device,
    const data::item::Device &data, uint32_t pos)
{
  // @formatter:off
  device.append_child("Id").text().set(data.getId());
  device.append_child("Type").text().set(data.type.get().getString());
  device.append_child("Manufacturer").text().set(data.mnf.get());
  device.append_child("Model").text().set(data.model.get());

  auto properties = device.append_child("Properties");
  writeProperty(properties, "AlwaysOn", c->getDevices()[pos].alwaysOn);
  writeProperty(properties, "AudioSwitch", c->getDevices()[pos].audioSwitch);
  writeProperty(properties, "AutoPower", c->getDevices()[pos].autoPower);
  writeProperty(properties, "Dimmer", c->getDevices()[pos].dimmer);
  writeProperty(properties, "HasBands", c->getDevices()[pos].hasBands);
  writeProperty(properties, "HasPresets", c->getDevices()[pos].hasPresets);
  writeProperty(properties, "IsNewDevice", c->getDevices()[pos].isNewDevice);
  writeProperty(properties, "IsDisplayDevice", c->getDevices()[pos].isDisplayDevice);
  writeProperty(properties, "ManualPower", c->getDevices()[pos].manualPower);
  writeProperty(properties, "MenuOnDevice", c->getDevices()[pos].menuOnDevice);
  writeProperty(properties, "OnScreenGuide", c->getDevices()[pos].onScreenGuide);
  writeProperty(properties, "NumDiscs", c->getDevices()[pos].numDiscs);
  writeProperty(properties, "NumLights", c->getDevices()[pos].numLights);
  writeProperty(properties, "PvrType", c->getDevices()[pos].pvrType);
  writeProperty(properties, "RecordMedia Fixed Disc", c->getDevices()[pos].recordMediaFixedDisc);
  writeProperty(properties, "RecordMedia Removable Videotape", c->getDevices()[pos].recordMediaRemovableVideotape);
  writeProperty(properties, "RevertInput", c->getDevices()[pos].revertInput);
  writeProperty(properties, "Scart", c->getDevices()[pos].scart);
  writeProperty(properties, "TunerInput", c->getDevices()[pos].tunerInput);
  writeProperty(properties, "VideoSwitch", c->getDevices()[pos].videoSwitch);
  for (const auto &prop : c->getDevices()[pos].getUnknownProperties()) {
    writeUnknownElement(properties, prop);
  }
  //todo weitere...

// @formatter:on
  return true;
}

bool ConfigH900::writeActivities(pugi::xml_node &root)
{
  return true;
}

bool ConfigH900::writeActivitiy(pugi::xml_node &activitie)
{
  return true;
}

bool ConfigH900::writeProtocols(pugi::xml_node &root)
{
  return true;
}

bool ConfigH900::writeProtocol(pugi::xml_node &protocol)
{
  return true;
}

void ConfigH900::writeUnknownElement(pugi::xml_node &parent,
    const data::item::UnknownElement &element)
{
  auto node = parent.append_child(element.tag.c_str());

  for (const auto& [name, value] : element.attributes) {
    node.append_attribute(name.c_str()) = value.c_str();
  }

  if (!element.text.empty()) {
    node.text().set(element.text.c_str());
  }

  for (const auto &child : element.children) {
    writeUnknownElement(node, child);
  }
}

}
}
