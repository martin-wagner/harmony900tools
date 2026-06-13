// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <pugixml.hpp>

#include "version.h"
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
    const Property<T> &prop, string childName = "Property")
{
  if (prop.isIncluded() != Include::ALWAYS) {
    return;
  }
  auto property = parent.append_child(childName);
  property.append_attribute("name").set_value(name);
  property.text().set(prop.get());
}

//not a class member!. for enum types
template<typename T>
void writeProperty(pugi::xml_node &parent, const char *name,
    const Property<Enum<T>> &prop, string childName = "Property")
{
  if (prop.isIncluded() != Include::ALWAYS) {
    return;
  }
  auto property = parent.append_child(childName);
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
  auto presentation = device.child("Presentation");
  auto label = presentation.child("Label").child_value();
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

  ret = true;
  for (pugi::xml_node prop : presentation.children("ControlGroup")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "Misc"_hash: {
        ret &= readButtons(prop, data::item::ButtonType::Soft);
        break;
      }
      case "HardButtons"_hash: {
        ret &= readButtons(prop, data::item::ButtonType::Hard);
        break;
      }
    }
  }
  auto states = device.child("States");
  ret = readStatemachines(states);

  if (!ret) {
    return ret;
  }

  //todo weitere

  return true;
}

bool ConfigH900::readButtons(pugi::xml_node &buttons,
    enum data::item::ButtonType t)
{
  auto ret = true;
  for (pugi::xml_node prop : buttons.children("Button")) {
    ret &= readButton(prop, t);
  }
  return ret;
}

bool ConfigH900::readButton(pugi::xml_node &button,
    enum data::item::ButtonType t)
{
  int buttonPos;

  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addButtonCommand(t, devicePos, -1); //append
  if (!ret) {
    return false;
  }
  if (t == data::item::ButtonType::Hard) {
    buttonPos = c->getDevices()[devicePos].getHardButtons().size() - 1;
  } else {
    buttonPos = c->getDevices()[devicePos].getSoftButtons().size() - 1;
  }

  auto actionId = string(
      button.child("ActionId").text().as_string("1_unknown_Hold"));
  auto action = data::item::Button::getAction(actionId);
  worker->setButtonAction(action, t, devicePos, buttonPos);

  //hard/soft are different
  auto name = button.attribute("name");
  if (name) {
    auto c = name.value();
    worker->setButtonName(string(c), t, devicePos, buttonPos);
  } else {
    auto c = button.child("Label").child_value();
    worker->setButtonName(string(c), t, devicePos, buttonPos);
  }

  //only soft buttons
  auto pos = button.child("Position");
  if (pos) {
    auto p = pos.text().as_uint(0);
    worker->setButtonPosition(p, t, devicePos, buttonPos);
  }
  auto file = button.child("Icon");
  if (file) {
    auto c = name.value();
    worker->setButtonFile(string(c), t, devicePos, buttonPos);
  }
  return true;
}

bool ConfigH900::readStatemachines(pugi::xml_node &states)
{
  auto ret = true;
  for (pugi::xml_node prop : states.children("State")) {
    ret &= readStatemachine(prop);
  }
  return ret;
}

bool ConfigH900::readStatemachine(pugi::xml_node &state)
{
  data::ActionClass ac = data::ActionClass::Unknown;

  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addStatemachineCommand(devicePos, -1); //append
  if (!ret) {
    return false;
  }
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto id = state.child("Id").text().as_string();
  worker->setStatemachineType(Enum<StateMachineType>(id), devicePos, smPos);
  //value -- don't store redundant data, is name of each action. we could check consistency...
  auto delay = state.child("Delay");
  if (delay) {
    worker->setStatemachineDelay(delay.text().as_uint(0), devicePos, smPos);
  }

  ret = readDeviceActions(state, ac);
  worker->setStatemachineActionClass(data::Enum<data::ActionClass>(ac),
      devicePos, smPos);
  return ret;
}

bool ConfigH900::readDeviceActions(pugi::xml_node &state, data::ActionClass &ac)
{
  auto ret = true;
  auto discreteActions = state.child("DiscreteActions");
  auto relativeActions = state.child("RelativeActions");
  if (!discreteActions.empty() && !relativeActions.empty()) {
    emit writeLog(LogLevel::Error,
        tr("xml: have DiscreteActions and RelativeActions "
            "at the same time in %1. This is not supported").arg(
            QString::fromStdString(c->getDevices().back().label.get())),
        ContentType::PlainText);
    return false;
  }

  //only one contains data
  for (pugi::xml_node prop : discreteActions.children()) {
    ac = data::ActionClass::DiscreteActions;
    ret &= readDeviceAction(prop);
  }
  for (pugi::xml_node prop : relativeActions.children()) {
    ac = data::ActionClass::RelativeActions;
    ret &= readDeviceAction(prop);
  }
  return ret;
}

bool ConfigH900::readDeviceAction(pugi::xml_node &actionType)
{
  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto ret = worker->addDeviceActionCommand(devicePos, smPos, -1); //append
  if (!ret) {
    return false;
  }
  auto actPos =
      c->getDevices()[devicePos].getStateMachines()[smPos].getActions().size()
          - 1;
  auto at = actionType.name();
  worker->setDeviceActionType(data::Enum<data::ActionType>(at), devicePos,
      smPos, actPos);
  auto name = actionType.child("Name").text().as_string();
  worker->setDeviceActionName(name, devicePos, smPos, actPos);
  auto action = actionType.child("Action");
  auto target = action.child("Target").text().as_string();
  if (string(target) != "Device") {
    emit writeLog(LogLevel::Warning,
        tr("xml: Action Target != Device in %1. This is not supported").arg(
            QString::fromStdString(c->getDevices().back().label.get())),
        ContentType::PlainText);
    //ignore
  }
  auto operation = action.child("Operation");
  auto opcode = operation.child("Name").text().as_string();
  worker->setDeviceActionOp(data::Enum<data::Operation>(opcode), devicePos,
      smPos, actPos);
  for (pugi::xml_node prop : operation.children("Parameter")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "DeviceId"_hash:
        //don't store redundant data
        break;
      case "Command"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceActionCmd(val, devicePos, smPos, actPos);
        break;
      }
      case "Modifier"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceActionMod(data::Enum<data::Modifier>(val), devicePos,
            smPos, actPos);
        break;
      }
      case "DelayValue"_hash: {
        auto val = prop.text().as_uint();
        worker->setDeviceActionDelayMs(val, devicePos, smPos, actPos);
        break;
      }
      case "State"_hash:
      case "StateName"_hash: {
        //state without value -> StateName. wtf???
        auto val = prop.text().as_string();
        worker->setDeviceActionStateName(
            data::Enum<data::StateMachineType>(val), devicePos, smPos, actPos);
        break;
      }
      case "Value"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceActionStateValue(val, devicePos, smPos, actPos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import device action: unknown property (value = %1)").arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
        worker->setDeviceActionUnknownParam(unknown, devicePos, smPos, actPos);
        break;
      }
    }
  }
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
  user.append_child("H900Version").text().set(BuildInfo::versionString); //mark as non-h900 software created
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
  for (const auto &data : devices) {
    auto device = root.append_child("Device");
    ret &= writeDevice(device, data);
  }
  return ret;
}

bool ConfigH900::writeDevice(pugi::xml_node &device,
    const data::item::Device &data)
{
  bool ret = true;

  // @formatter:off
  device.append_child("Id").text().set(data.getId());
  device.append_child("Type").text().set(data.type.get().getString());
  device.append_child("Manufacturer").text().set(data.mnf.get());
  device.append_child("Model").text().set(data.model.get());

  auto presentation = device.append_child("Presentation");
  presentation.append_child("Label").text().set(data.label.get());
  auto softButtons = presentation.append_child("ControlGroup");
  softButtons.append_attribute("name").set_value("Misc");
  ret &= writeButtons(softButtons, data.getId(), data.getSoftButtons());
  auto hardButtons = presentation.append_child("ControlGroup");
  hardButtons.append_attribute("name").set_value("HardButtons");
  ret &= writeButtons(hardButtons, data.getId(), data.getHardButtons());

  auto properties = device.append_child("Properties");
  writeProperty(properties, "AlwaysOn", data.alwaysOn);
  writeProperty(properties, "AudioSwitch", data.audioSwitch);
  writeProperty(properties, "AutoPower", data.autoPower);
  writeProperty(properties, "Dimmer", data.dimmer);
  writeProperty(properties, "HasBands", data.hasBands);
  writeProperty(properties, "HasPresets", data.hasPresets);
  writeProperty(properties, "IsNewDevice", data.isNewDevice);
  writeProperty(properties, "IsDisplayDevice", data.isDisplayDevice);
  writeProperty(properties, "ManualPower", data.manualPower);
  writeProperty(properties, "MenuOnDevice", data.menuOnDevice);
  writeProperty(properties, "OnScreenGuide", data.onScreenGuide);
  writeProperty(properties, "NumDiscs", data.numDiscs);
  writeProperty(properties, "NumLights", data.numLights);
  writeProperty(properties, "PvrType", data.pvrType);
  writeProperty(properties, "RecordMedia Fixed Disc", data.recordMediaFixedDisc);
  writeProperty(properties, "RecordMedia Removable Videotape", data.recordMediaRemovableVideotape);
  writeProperty(properties, "RevertInput", data.revertInput);
  writeProperty(properties, "Scart", data.scart);
  writeProperty(properties, "TunerInput", data.tunerInput);
  writeProperty(properties, "VideoSwitch", data.videoSwitch);
  for (const auto &prop : data.getUnknownProperties()) {
    writeUnknownElement(properties, prop);
  }

  auto states = device.append_child("States");
  ret &= writeStatemachines(states, data.getId(), data.getStateMachines());

  //todo weitere...

// @formatter:on
  return ret;
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

bool ConfigH900::writeButtons(pugi::xml_node &buttons, uint32_t deviceId,
    const vector<data::item::Button> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    auto button = buttons.append_child("Button");
    ret &= writeButton(button, deviceId, d);
  }
  return ret;
}

bool ConfigH900::writeButton(pugi::xml_node &button, uint32_t deviceId,
    const data::item::Button &data)
{
  if (data.getButtonType() == data::item::ButtonType::Hard) {
    button.append_attribute("name").set_value(data.name.get());
    button.append_child("Label"); //empty
  } else {
    button.append_child("Label").text().set(data.name.get());
  }
  if (data.position.isIncluded() == data::Include::ALWAYS) {
    button.append_child("Position").text().set(data.position.get());
  }
  button.append_child("ActionId").text().set(data.getActionId(deviceId));
  return true;
}

bool ConfigH900::writeStatemachines(pugi::xml_node &states, uint32_t deviceId,
    const vector<data::item::StateMachine> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    auto state = states.append_child("State");
    ret &= writeStatemachine(state, deviceId, d);
  }
  return ret;
}

bool ConfigH900::writeStatemachine(pugi::xml_node &state, uint32_t deviceId,
    const data::item::StateMachine &data)
{
  state.append_child("Id").text().set(data.smType.get().getString());
  for (const auto &d : data.getActions()) {
    state.append_child("Value").text().set(d.name.get());
  }
  if (data.delayMs.isIncluded() == data::Include::ALWAYS) {
    state.append_child("Delay").text().set(data.delayMs.get());
  }
  auto actionClass = state.append_child(data.actionClass.get().getString());
  return writeDeviceActions(actionClass, deviceId, data.getActions());
}

bool ConfigH900::writeDeviceActions(pugi::xml_node &actionClass,
    uint32_t deviceId, const vector<data::item::DeviceAction> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    auto actionType = actionClass.append_child(d.actionType.get().getString());
    auto action = actionType.append_child("Action");
    ret &= writeDeviceAction(action, deviceId, d);
    actionType.append_child("Name").text().set(d.name.get());
  }
  return ret;
}

bool ConfigH900::writeDeviceAction(pugi::xml_node &action, uint32_t deviceId,
    const data::item::DeviceAction &data)
{
  action.append_child("Target").text().set("Device"); //static value
  auto operation = action.append_child("Operation");
  operation.append_child("Name").text().set(data.op.get().getString());
  auto id = operation.append_child("Parameter");
  id.append_attribute("name").set_value("DeviceId");
  id.text().set(deviceId);
  writeProperty(operation, "Command", data.cmd, "Parameter");
  writeProperty(operation, "Modifier", data.mod, "Parameter");
  writeProperty(operation, "DelayValue", data.delayMs, "Parameter");
  if (data.stateValue.isIncluded() == data::Include::ALWAYS) {
    writeProperty(operation, "State", data.stateName, "Parameter");
    writeProperty(operation, "Value", data.stateValue, "Parameter");
  } else {
    //state without value -> StateName. wtf???
    writeProperty(operation, "StateName", data.stateName, "Parameter");
  }
  for (const auto &prop : data.getUnknownParams()) {
    writeUnknownElement(operation, prop);
  }
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
