// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <pugixml.hpp>

#include "version.h"
#include "lib/qtHelpers.h"
#include "lib/consthash.h"
#include "lib/uid.h"
#include "lib/timestamp.h"
#include "lib/harmony_endian.h"
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
  if (prop.isIncluded() != Used::YES) {
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
  if (prop.isIncluded() != Used::YES) {
    return;
  }
  auto property = parent.append_child(childName);
  property.append_attribute("name").set_value(name);
  property.text().set(prop.get().getString().c_str());
}

H900userconfig::H900userconfig(const QString &workPath) :
    wp(workPath)
{
}

void H900userconfig::usePrettyPrinting(bool v)
{
  prettyPrinting = v;
}

bool H900userconfig::dump(const ConfigData *c)
{
  bool ret = true;
  int pugiFormat = pugi::format_raw;
  vector<uint8_t> tmp;

  this->c = c;
  this->worker = nullptr;
  writerTime = lib::writeTimeH900Xml();
  streams = binary::ssIr::File();

  if (prettyPrinting) {
    pugiFormat = pugi::format_default;
  }

  emit writeLog(LogLevel::Info, tr("Exporting data..."),
      ContentType::PlainText);

  QDir().mkpath(wp + "/" + QFileInfo(userConfigPath).path());
  try {
    ret &= writeIrProto(); //side effect: creates hash for xml files
    ret &= dumpUserConfigXml(pugiFormat);
    ret &= dumpActionListXml(pugiFormat);
    ret &= writeIrStream();
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("export: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
    ret = false;
  }

  this->c = nullptr;

  if (ret == true) {
    emit writeLog(LogLevel::Info, tr("Export successful"),
        ContentType::PlainText);
  }
  return ret;
}

bool H900userconfig::read(const ConfigData *c, CmdCatalogue *worker)
{
  bool ret = true;

  this->c = c;
  this->worker = worker;

  emit writeLog(LogLevel::Info, tr("Importing user config..."),
      ContentType::PlainText);

  try {
    ret &= readIrStream();
    ret &= readUserConfigXml();
    ret &= readIrProto();
  } catch (const exception &e) {
    emit writeLog(LogLevel::Error,
        tr("import: exception: %1").arg(QString(e.what())),
        ContentType::PlainText);
    ret = false;
  }

  this->c = nullptr;
  this->worker = nullptr;

  if (ret == true) {
    emit writeLog(LogLevel::Info, tr("Import successful"),
        ContentType::PlainText);
  }
  return ret;
}

bool H900userconfig::readUserConfigXml()
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

  auto root = xml.child("Root");
  auto ret = readProperties(root); //version check
  if (!ret) {
    emit writeLog(LogLevel::Error, tr("Importing xml failed (header)"),
        ContentType::PlainText);
    return ret;
  }
  ret &= readUser(root);
  ret &= readController(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("Importing xml failed (user)"),
        ContentType::PlainText);
    return false;
  }
  ret = readDevices(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("Importing xml failed (devices)"),
        ContentType::PlainText);
    return false;
  }
  ret = readActivities(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("Importing xml failed (activities)"),
        ContentType::PlainText);
    return false;
  }
  emit writeLog(LogLevel::Debug,
      tr("Importing xml successful, contains %1 devices and %2 activities").arg(
          c->getDevices().size()).arg(c->getActivities().size()),
      ContentType::PlainText);
  return true;
}

bool H900userconfig::readProperties(pugi::xml_node &root)
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
        emit writeLog(LogLevel::Debug, tr("import: irproto crc %1").arg(s),
            ContentType::PlainText);
        break;
      }
      case "LastUpdated"_hash: {
        QString s = prop.child_value();
        emit writeLog(LogLevel::Info, tr("import: last update at %1").arg(s),
            ContentType::PlainText);
        break;
      }
    }
  }
  return true;
}

bool H900userconfig::readUser(pugi::xml_node &root)
{
  auto user = root.child("User");

  auto id = user.child("Id").text().as_uint();
  addId(id);
  worker->setUserId(id);
  auto firstName = user.child("Presentation").child("FirstName").child_value();
  worker->setUserFirstName(qstr(firstName));
  auto lastName = user.child("Presentation").child("LastName").child_value();
  worker->setUserLastName(qstr(lastName));
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
            tr("import user: unknown property (name = %1, value = %2)").arg(
                qstr(name)).arg(qstr(unknown.text)), ContentType::PlainText);
        worker->setUserUnknownProperty(unknown);
        break;
      }
    }
  }
  return true;
}

bool H900userconfig::readController(pugi::xml_node &root)
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
            qstr(unknown.text)), ContentType::PlainText);
    worker->setControllerUnknownProperty(unknown);
  }
  return true;
}

bool H900userconfig::readDevices(pugi::xml_node &root)
{
  bool ret = true;

  emit writeLog(LogLevel::Info, tr("Reading devices..."),
      ContentType::PlainText);

  for (pugi::xml_node device : root.children("Device")) {
    ret &= readDevice(device);
  }
  return ret;
}

bool H900userconfig::readDevice(pugi::xml_node &device)
{
  auto id = device.child("Id").text().as_uint();
  addId(id);
  auto ret = worker->addDeviceCommand(-1, id); //append
  if (!ret) {
    return false;
  }
  auto pos = c->getDevices().size() - 1;
  emit writeLog(LogLevel::Info, tr("Device %1...").arg(pos),
      ContentType::PlainText);

  auto typeStr = device.child("Type").child_value();
  Enum<DeviceType> type(typeStr);
  worker->setDeviceType(type, pos);
  auto mnf = device.child("Manufacturer").child_value();
  worker->setDeviceMnf(mnf, pos);
  auto model = device.child("Model").child_value();
  worker->setDeviceModel(model, pos);
  auto presentation = device.child("Presentation");
  auto label = presentation.child("Label").child_value();
  worker->setDeviceLabel(label, pos);

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
            tr("import device: unknown property (name = %1, value = %2)").arg(
                qstr(name)).arg(qstr(unknown.text)), ContentType::PlainText);
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
        ret &= readDeviceSoftButtons(prop);
        break;
      }
      case "HardButtons"_hash: {
        ret &= readDeviceHardButtons(prop);
        break;
      }
    }
  }
  auto states = device.child("States");
  ret &= readStatemachines(states);
  auto numeric = device.child("Numeric");
  if (numeric) {
    ret &= readNumeric(numeric);
  }
  auto commands = device.child("Commands");
  ret &= readIrList(commands);
  return ret;
}

bool H900userconfig::readDeviceHardButtons(pugi::xml_node &buttons)
{
  auto ret = true;
  for (pugi::xml_node prop : buttons.children("Button")) {
    ret &= readDeviceHardButton(prop);
  }
  return ret;
}

bool H900userconfig::readDeviceHardButton(pugi::xml_node &button)
{
  int buttonPos;
  auto t = item::ButtonType::Hard;
  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addDeviceButtonCommand(devicePos, t, -1); //append
  if (!ret) {
    return false;
  }

  buttonPos = c->getDevices()[devicePos].getHardButtons().size() - 1;

  auto actionId = string(button.child("ActionId").text().as_string());
  if (actionId.empty()) {
    return false;
  }
  auto action = item::Button::getAction(actionId);
  worker->setDeviceButtonAction(action, devicePos, t, buttonPos);
  auto name = button.attribute("name").value();
  worker->setDeviceButtonName(string(name), devicePos, t, buttonPos);
  return true;
}

bool H900userconfig::readDeviceSoftButtons(pugi::xml_node &buttons)
{
  auto ret = true;
  std::map<int, pugi::xml_node> buttonsByPos;
  int maxPos = -1;
  auto devicePos = c->getDevices().size() - 1;

  for (pugi::xml_node button : buttons.children("Button")) {
    auto uiPos = button.child("Position").text().as_int(-1);
    if (uiPos < 0) {
      return false;
    }
    buttonsByPos[uiPos] = button;
    if (uiPos > maxPos) {
      maxPos = uiPos;
    }
  }

  for (int i = 0; i <= maxPos; i++) {
    auto it = buttonsByPos.find(i);
    if (it == buttonsByPos.end()) {
      ret &= addDummyDeviceSoftButton(devicePos, i);
    } else {
      ret &= readDeviceSoftButton(it->second, devicePos, i);
    }
  }
  return ret;
}

bool H900userconfig::readDeviceSoftButton(pugi::xml_node &button, int devicePos,
    int uiPos)
{
  auto t = item::ButtonType::Soft;

  auto ret = worker->addDeviceButtonCommand(devicePos, t, uiPos);
  if (!ret) {
    return false;
  }
  auto actionId = string(button.child("ActionId").text().as_string());
  if (actionId.empty()) {
    return false;
  }
  auto action = item::Button::getAction(actionId);
  worker->setDeviceButtonAction(action, devicePos, t, uiPos);
  auto label = button.child("Label").child_value();
  worker->setDeviceButtonName(string(label), devicePos, t, uiPos);
  worker->setDeviceButtonPosition(uiPos, devicePos, t, uiPos);
  auto file = button.child("Icon");
  if (file) {
    auto c = file.text().as_string();
    worker->setDeviceButtonFile(string(c), devicePos, t, uiPos);
  }
  return true;
}

bool H900userconfig::addDummyDeviceSoftButton(int devicePos, int pos)
{
  auto t = item::ButtonType::Soft;

  auto ret = worker->addDeviceButtonCommand(devicePos, t, pos);
  if (!ret) {
    return false;
  }
  worker->setDeviceButtonAction(string(item::Button::UNUSED), devicePos, t,
      pos);
  worker->setDeviceButtonName("", devicePos, t, pos);
  worker->setDeviceButtonPosition(pos, devicePos, t, pos);
  return true;
}

bool H900userconfig::readStatemachines(pugi::xml_node &states)
{
  auto ret = true;
  for (pugi::xml_node prop : states.children("State")) {
    ret &= readStatemachine(prop);
  }
  return ret;
}

bool H900userconfig::readStatemachine(pugi::xml_node &state)
{
  bool ret = true;

  auto devicePos = c->getDevices().size() - 1;

  ret &= worker->addDeviceStatemachineCommand(devicePos, -1); //append
  if (!ret) {
    return false;
  }
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto id = state.child("Id").text().as_string();
  worker->setDeviceStatemachineType(Enum<StateMachineDeviceType>(id), devicePos,
      smPos);
  auto delay = state.child("Delay");
  if (delay) {
    worker->setDeviceStatemachineDelay(delay.text().as_uint(0), devicePos,
        smPos);
  }

  ret &= readStartAction(state);
  ret &= readFinishAction(state);
  auto discreteActions = state.child("DiscreteActions");
  auto relativeActions = state.child("RelativeActions");
  if (!discreteActions.empty() && !relativeActions.empty()) {
    emit writeLog(LogLevel::Error,
        tr("xml: have DiscreteActions and RelativeActions "
            "at the same time in %1. This is not supported").arg(
            qstr(c->getDevices().back().label.get())), ContentType::PlainText);
    return false;
  }
  ret &= readDiscreteActions(discreteActions);
  ret &= readRelativeActions(state);
  return ret;
}

bool H900userconfig::readStartAction(pugi::xml_node &state)
{
  auto action = state.child("StartAction");
  if (action.empty()) {
    return true;
  }
  return readGeneralAction(action, item::StateMachineAction::Start);
}

bool H900userconfig::readFinishAction(pugi::xml_node &state)
{
  auto action = state.child("FinishAction");
  if (action.empty()) {
    return true;
  }
  return readGeneralAction(action, item::StateMachineAction::Finish);
}

bool H900userconfig::readGeneralAction(pugi::xml_node &action,
    item::StateMachineAction t)
{
  item::DeviceAction out;

  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto ret = worker->addDeviceSmActionCommand(devicePos, smPos, t);
  if (!ret) {
    return false;
  }

  out.actionType.set(Enum<ActionType>(action.name())).setIncluded(Used::YES);
  ret = readActionSequences(action, out);
  if (!ret) {
    return false;
  }
  return worker->setDeviceSmAction(out, devicePos, smPos, t, 0);
}

bool H900userconfig::readDiscreteActions(pugi::xml_node &actions)
{
  auto ret = true;

  if (actions.children().empty()) {
    return true;
  }

  for (pugi::xml_node prop : actions.children()) {
    ret &= readDiscreteAction(prop);
  }
  return ret;
}

bool H900userconfig::readDiscreteAction(pugi::xml_node &action)
{
  item::DeviceAction out;

  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto name = QString(action.child("Name").text().as_string());
  if (name.isEmpty()) {
    emit writeLog(LogLevel::Error,
        tr("xml: state name is empty in %1").arg(
            qstr(c->getDevices().back().label.get())), ContentType::PlainText);
    return false;
  }

  auto ret = worker->addDeviceSmStateCommand(devicePos, smPos,
      item::StateMachineType::Discrete, name, -1); //append
  if (!ret) {
    return false;
  }
  auto actPos =
      c->getDevices()[devicePos].getStateMachines()[smPos].discrete.states.size()
          - 1;

  out.actionType.set(Enum<ActionType>(action.name())).setIncluded(Used::YES);
  ret = readActionSequences(action, out);
  if (!ret) {
    return false;
  }
  return worker->setDeviceSmAction(out, devicePos, smPos,
      item::StateMachineAction::Discrete_Enter, actPos);
}

bool H900userconfig::readRelativeActions(pugi::xml_node &state)
{
  auto ret = true;

  if (!state.child("RelativeActions")) {
    return true;
  }

  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  //relative actions consists of state names and transition actions as separate items.
  //state names
  for (pugi::xml_node prop : state.children("Value")) {
    auto name = prop.text().as_string();
    ret &= worker->addDeviceSmStateCommand(devicePos, smPos,
        item::StateMachineType::Relative, name, -1); //append
  }
  if (!ret) {
    return false;
  }

  //transition actions
  for (pugi::xml_node prop : state.child("RelativeActions").children()) {
    ret &= readRelativeAction(prop);
  }
  return ret;
}

bool H900userconfig::readRelativeAction(pugi::xml_node &action)
{
  item::DeviceAction out;

  item::StateMachineAction add;
  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto type = Enum<ActionType>(action.name());
  switch (type.getValue()) {
    case ActionType::NextAction:
      add = item::StateMachineAction::Relative_Next;
      break;
    case ActionType::PrevAction:
      add = item::StateMachineAction::Relative_Prev;
      break;
    case ActionType::ResetAction:
      add = item::StateMachineAction::Relative_Reset;
      break;
    default:
      emit writeLog(LogLevel::Error,
          tr("xml: action type %1 unknown").arg(type.getQString()),
          ContentType::PlainText);
      return false;
  }
  auto ret = worker->addDeviceSmActionCommand(devicePos, smPos, add);
  if (!ret) {
    return false;
  }

  out.actionType.set(Enum<ActionType>(action.name())).setIncluded(Used::YES);
  ret = readActionSequences(action, out);
  if (!ret) {
    return false;
  }
  return worker->setDeviceSmAction(out, devicePos, smPos, add, 0);
}

bool H900userconfig::readActionSequences(pugi::xml_node &action,
    item::DeviceAction &out)
{
  auto ret = true;

  for (pugi::xml_node prop : action.children("Action")) {
    ret &= readActionSequence(prop, out);
  }
  return ret;
}

bool H900userconfig::readActionSequence(pugi::xml_node &sequence,
    item::DeviceAction &out)
{
  item::SequenceItem seq;

  auto target = sequence.child("Target").text().as_string();
  if (string(target) != "Device") {
    emit writeLog(LogLevel::Warning,
        tr("xml: Action Target != Device in %1. This is not supported").arg(
            qstr(c->getDevices().back().label.get())), ContentType::PlainText);
    return false;
  }
  auto operation = sequence.child("Operation");
  seq.opcode.set(operation.child("Name").text().as_string());
  for (pugi::xml_node prop : operation.children("Parameter")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "Id"_hash:
        seq.deviceId.set(prop.text().as_uint()).setIncluded(Used::YES);
        break;
      case "DeviceId"_hash:
        //don't store redundant data
        break;
      case "Command"_hash:
        seq.cmd.set(prop.text().as_string()).setIncluded(Used::YES);
        break;
      case "Modifier"_hash:
        seq.mod.set(prop.text().as_string()).setIncluded(Used::YES);
        break;
      case "DelayValue"_hash:
        seq.delayMs.set(prop.text().as_uint()).setIncluded(Used::YES);
        break;
      case "State"_hash:
      case "StateName"_hash:
        seq.stateName.set(prop.text().as_string()).setIncluded(Used::YES);
        break;
      case "Value"_hash:
        seq.value.set(prop.text().as_string()).setIncluded(Used::YES);
        break;
      default: {
        auto unknown = toUnknownElement(prop);
        seq.getUnknownParams().push_back(unknown);
        emit writeLog(LogLevel::Debug,
            tr("import device action: unknown property (name = %1, value = %2)").arg(
                qstr(name)).arg(qstr(unknown.text)), ContentType::PlainText);
        break;
      }
    }
  }
  out.sequence.push_back(seq);
  return true;
}

bool H900userconfig::readNumeric(pugi::xml_node &numeric)
{
  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addDeviceNumpadCommand(devicePos);
  if (!ret) {
    return false;
  }
  auto digits = numeric.child("FixedDigits");
  if (digits) {
    worker->setDeviceNumpadFixedDigits(digits.text().as_uint(), devicePos);
  }

  ret = true;
  auto firstDigit = numeric.child("FirstDigit");
  if (firstDigit) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::First);
    ret &= readNumericActions(firstDigit, item::DigitSection::First);
  }
  auto middleDigit = numeric.child("MiddleDigit");
  if (middleDigit) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::Middle);
    ret &= readNumericActions(middleDigit, item::DigitSection::Middle);
  }
  auto lastDigit = numeric.child("LastDigit");
  if (lastDigit) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::Last);
    ret &= readNumericActions(lastDigit, item::DigitSection::Last);
  }

  auto start = numeric.child("Start");
  if (start) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::Start);
    ret &= readNumericActionSequences(start, devicePos,
        item::DigitSection::Start, 0);
  }
  auto greaterTen = numeric.child("GreaterTen");
  if (greaterTen) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::GreaterTen);
    ret &= readNumericActionSequences(greaterTen, devicePos,
        item::DigitSection::GreaterTen, 0);
  }
  auto greaterHundred = numeric.child("GreaterHundred");
  if (greaterHundred) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::GreaterHundred);
    ret &= readNumericActionSequences(greaterHundred, devicePos,
        item::DigitSection::GreaterHundred, 0);
  }
  auto finish = numeric.child("Finish");
  if (finish) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::Finish);
    ret &= readNumericActionSequences(finish, devicePos,
        item::DigitSection::Finish, 0);
  }
  return ret;
}

bool H900userconfig::readNumericActions(pugi::xml_node &actions,
    item::DigitSection s)
{
  uint32_t i;
  auto ret = true;
  auto devicePos = c->getDevices().size() - 1;

  if (actions.children().empty()) {
    return true;
  }
  if (std::distance(actions.children().begin(), actions.children().end())
      != item::Digits().size()) {
    emit writeLog(LogLevel::Error,
        tr("xml: numpad items != %1 in %2").arg(item::Digits().size()).arg(
            qstr(c->getDevices().back().label.get())), ContentType::PlainText);
    return false;
  }

  i = 0;
  for (pugi::xml_node prop : actions.children()) {
    //ignore "Digit value = x"
    ret &= readNumericActionSequences(prop, devicePos, s, i);
    i++;
  }
  return ret;
}

bool H900userconfig::readNumericActionSequences(pugi::xml_node &action,
    uint32_t devicePos, item::DigitSection s, uint32_t digit)
{
  auto ret = true;

  for (pugi::xml_node prop : action.children("Action")) {
    ret &= readNumericActionSequence(prop, devicePos, s, digit);
  }
  return ret;
}

bool H900userconfig::readNumericActionSequence(pugi::xml_node &sequence,
    uint32_t devicePos, item::DigitSection s, uint32_t digit)
{
  uint32_t seqPos;

  worker->addDeviceNumpadActionSequenceCommand(devicePos, s, digit, -1); //append
  auto &num = c->getDevices()[devicePos].getNumpad().value();
  switch (s) {
    case item::DigitSection::First:
      seqPos = num.first->at(digit).sequence.size() - 1;
      break;
    case item::DigitSection::Middle:
      seqPos = num.middle->at(digit).sequence.size() - 1;
      break;
    case item::DigitSection::Last:
      seqPos = num.last->at(digit).sequence.size() - 1;
      break;
    default:
      seqPos = 0;
      break;
  }
  return readActionSequenceData(sequence, devicePos, s, digit, seqPos);
}

bool H900userconfig::readActionSequenceData(pugi::xml_node &sequence,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  auto target = sequence.child("Target").text().as_string();
  if (string(target) != "Device") {
    emit writeLog(LogLevel::Warning,
        tr("xml: Action Target != Device in %1. This is not supported").arg(
            qstr(c->getDevices().back().label.get())), ContentType::PlainText);
    return false;
  }
  auto operation = sequence.child("Operation");
  auto opcode = operation.child("Name").text().as_string();
  worker->setDeviceNumpadActionSequenceOp(Enum<Operation>(opcode), devicePos, s,
      digit, seqPos);
  for (pugi::xml_node prop : operation.children("Parameter")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "DeviceId"_hash:
        //don't store redundant data
        break;
      case "Command"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceNumpadActionSequenceCmd(val, devicePos, s, digit,
            seqPos);
        break;
      }
      case "Modifier"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceNumpadActionSequenceMod(Enum<Modifier>(val), devicePos,
            s, digit, seqPos);
        break;
      }
      case "DelayValue"_hash: {
        auto val = prop.text().as_uint();
        worker->setDeviceNumpadActionSequenceDelayMs(val, devicePos, s, digit,
            seqPos);
        break;
      }
      case "State"_hash:
      case "StateName"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceNumpadActionSequenceStateName(
            Enum<StateMachineDeviceType>(val), devicePos, s, digit, seqPos);
        break;
      }
      case "Value"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceNumpadActionSequenceValue(val, devicePos, s, digit,
            seqPos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import device action: unknown property (name = %1, value = %2)").arg(
                qstr(name)).arg(qstr(unknown.text)), ContentType::PlainText);
        worker->setDeviceNumpadActionUnknownParam(unknown, devicePos, s, digit,
            seqPos);
        break;
      }
    }
  }
  return true;
}

bool H900userconfig::readIrList(pugi::xml_node &commands)
{
  auto devicePos = c->getDevices().size() - 1;

  for (pugi::xml_node prop : commands.child("Properties").children("Property")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "PressPreSilence"_hash: {
        auto val = prop.text().as_uint(0);
        worker->setIrPressPreSilenceMs(val, devicePos);
        break;
      }
      case "PressInterKey"_hash: {
        auto val = prop.text().as_uint(0);
        worker->setIrPressInterKeyMs(val, devicePos);
        break;
      }
      case "HoldPreSilence"_hash: {
        auto val = prop.text().as_uint(0);
        worker->setIrHoldPreSilenceMs(val, devicePos);
        break;
      }
      case "HoldInterKey"_hash: {
        auto val = prop.text().as_uint(0);
        worker->setIrHoldInterKeyMs(val, devicePos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import device: unknown property (name = %1, value = %2)").arg(
                qstr(name)).arg(qstr(unknown.text)), ContentType::PlainText);
        worker->setIrUnknownProperty(unknown, devicePos);
        break;
      }
    }
  }

  auto ret = true;
  for (pugi::xml_node prop : commands.children("Command")) {
    ret &= readIr(prop);
  }
  return ret;
}

bool H900userconfig::readIr(pugi::xml_node &command)
{
  auto devicePos = c->getDevices().size() - 1;

  auto name = command.child("Name").child_value();
  auto data = command.child("Data");
  auto protoIndex = data.child("Protocol").text().as_int(); // -1 -> escape raw cmd
  auto code = string(data.child("Code").child_value());
  auto rawData = lib::hexStringToBytes(code);
  if (protoIndex < 0) {
    //raw command
    if (rawData.size() != 4) {
      emit writeLog(LogLevel::Warning,
          tr("xml: Raw cmd size != 4 (%1). This is not supported.").arg(
              rawData.size()), ContentType::PlainText);
      return false;
    }
    rawData.erase(rawData.begin(), rawData.begin() + 2); //remove 0xffff

    item::RawCommand cmd;
    cmd.name.set(name);
    cmd.stream = streams.accessStream(
        lib::parseHarmony16_file(rawData[0], rawData[1]));
    return worker->setIrCommand(devicePos, cmd);
  } else {
    //protocol command
    auto cmd = item::ProtoCommand(protoIndex, rawData);
    if (cmd.getStatus() != binary::irProto::Status::OK) {
      emit writeLog(LogLevel::Debug,
          tr("import device: device %1 cmd %2 decode failed (%3). "
              "Using copy-trough)").arg(devicePos).arg(qstr(name)).arg(
              (int) cmd.getStatus()), ContentType::PlainText);
    }
    cmd.name.set(name);
    return worker->setIrCommand(devicePos, cmd);
  }
}

bool H900userconfig::readActivities(pugi::xml_node &root)
{
  auto ret = true;

  emit writeLog(LogLevel::Info, tr("Reading activities..."),
      ContentType::PlainText);

  for (pugi::xml_node prop : root.children("Activity")) {
    auto id = prop.child("Id").text().as_int();
    if (id == -1) {
      //power off activity, createt from other data.
      continue;
    }
    ret &= readActivitiy(prop, id);
  }
  return ret;
}

bool H900userconfig::readActivitiy(pugi::xml_node &activity, uint32_t id)
{
  addId(id);
  auto ret = worker->addActivityCommand(-1, id); //append
  if (!ret) {
    return false;
  }
  auto pos = c->getActivities().size() - 1;
  emit writeLog(LogLevel::Info, tr("Activity %1...").arg(pos),
      ContentType::PlainText);

  auto typeStr = activity.child("Type").child_value();
  Enum<ActivityType> type(typeStr);
  worker->setActivityType(type, pos);
  auto presentation = activity.child("Presentation");
  auto label = presentation.child("Label").child_value();
  worker->setActivityLabel(label, pos);

  for (pugi::xml_node prop : activity.child("Properties").children("Property")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());

    switch (h) {
      case "ActivityStartPage"_hash: {
        auto valStr = prop.text().as_string("Transport");
        auto val = Enum<ActivityStartPage>(valStr);
        worker->setActivityPvrType(val, pos);
        break;
      }
      case "ControlGroup_Hard Buttons"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityControlGroupHardButtons(val, pos);
        break;
      }
      case "PowerOffUnusedDevices"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityPowerOffUnusedDevices(val, pos);
        break;
      }
      case "TrainingWheels"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityTrainingWheels(val, pos);
        break;
      }
      case "UnusedDevicesHelp"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityUnusedDevicesHelp(val, pos);
        break;
      }
      case "ChannelButtonBehaviour"_hash: {
        auto valStr = prop.text().as_string("BasicChannels");
        auto val = Enum<ChannelButtonBehaviour>(valStr);
        worker->setActivityChannelButtonBehaviour(val, pos);
        break;
      }
      case "ControlGroup_Soft Buttons"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityControlGroupSoftButtons(val, pos);
        break;
      }
      case "EnableSmartMenu"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityEnableSmartMenu(val, pos);
        break;
      }
      case "EnableSmartZoom"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityEnableSmartZoom(val, pos);
        break;
      }
      case "GuideButtonMode"_hash: {
        auto valStr = prop.text().as_string("TunerProgramGuide");
        auto val = Enum<GuideButtonMode>(valStr);
        worker->setActivityGuideButtonMode(val, pos);
        break;
      }
      case "HideModeControl"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideModeControl(val, pos);
        break;
      }
      case "HideModeListen"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideModeListen(val, pos);
        break;
      }
      case "HideModeNavigate"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideModeNavigate(val, pos);
        break;
      }
      case "HideModePlay"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideModePlay(val, pos);
        break;
      }
      case "HideModePlayMode"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideModePlayMode(val, pos);
        break;
      }
      case "HideSurfAllChannels"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideSurfAllChannels(val, pos);
        break;
      }
      case "HideSurfAllShows"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideSurfAllShows(val, pos);
        break;
      }
      case "HideSurfFavoriteChannels"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideSurfFavoriteChannels(val, pos);
        break;
      }
      case "HideSurfFavoriteShows"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityHideSurfFavoriteShows(val, pos);
        break;
      }
      case "MaxTvContentDays"_hash: {
        auto val = prop.text().as_int(0);
        worker->setActivityMaxTvContentDays(val, pos);
        break;
      }
      case "MediaButtonMode"_hash: {
        auto valStr = prop.text().as_string("ShowMedia");
        auto val = Enum<MediaButtonMode>(valStr);
        worker->setActivityMediaButtonMode(val, pos);
        break;
      }
      case "PlayOnEnter"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityPlayOnEnter(val, pos);
        break;
      }
      case "RetainStop"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityRetainStop(val, pos);
        break;
      }
      case "ScrollChannelsByPage"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityScrollChannelsByPage(val, pos);
        break;
      }
      case "ScrollShowsByPage"_hash: {
        auto val = prop.text().as_bool(true);
        worker->setActivityScrollShowsByPage(val, pos);
        break;
      }
      case "StopOnExit"_hash: {
        auto val = prop.text().as_bool(false);
        worker->setActivityStopOnExit(val, pos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import activity: unknown property (name = %1, value = %2)").arg(
                qstr(name)).arg(qstr(unknown.text)), ContentType::PlainText);
        worker->setDeviceUnknownProperty(unknown, pos);
        break;
      }
    }
  }

  ret = true;
  auto channels = presentation.child("ChannelList");
  ret &= readActivityChannels(channels);
  for (pugi::xml_node prop : presentation.children("ControlGroup")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "Misc"_hash: {
        ret &= readActivitySoftButtons(prop);
        break;
      }
      case "HardButtons"_hash: {
        ret &= readActivityHardButtons(prop);
        break;
      }
      default:
        emit writeLog(LogLevel::Debug,
            tr("import activity: unknown button type (name = %1)").arg(
                qstr(name)), ContentType::PlainText);
        break;
    }
  }

  auto enter = activity.child("EnterActions");
  if (enter) {
    ret &= readActivityAction(enter, item::ActivityAction::Enter);
  }
  auto leave = activity.child("LeaveActions");
  if (leave) {
    ret &= readActivityAction(leave, item::ActivityAction::Leave);
  }
  ret &= readActivityRoles(activity);
  auto power = activity.child("Power");
  ret &= readActivityPowerStateDevices(power);
  return ret;
}

bool H900userconfig::readActivityChannels(pugi::xml_node &channels)
{
  auto ret = true;
  for (pugi::xml_node prop : channels.children("Channel")) {
    ret &= readActivityChannel(prop);
  }
  return ret;
}

bool H900userconfig::readActivityChannel(pugi::xml_node &channel)
{
  int channelPos;

  auto activityPos = c->getActivities().size() - 1;

  auto ret = worker->addActivityChannelCommand(activityPos, -1); //append
  if (!ret) {
    return false;
  }
  channelPos = c->getActivities()[activityPos].getChannels().size() - 1;

  auto station = string(channel.child("Station").text().as_string("Other"));
  worker->setActivityChannelStation(station, activityPos, channelPos);
  auto chNumber = channel.child("Number").text().as_uint(1);
  worker->setActivityChannelNumber(chNumber, activityPos, channelPos);
  auto chPosOnLcd = channel.child("Slot").text().as_uint(1);
  worker->setActivityChannelPosition(chPosOnLcd, activityPos, channelPos);
  auto image = string(channel.child("Image").text().as_string("0.png"));
  worker->setActivityChannelImage(image, activityPos, channelPos);
  return true;
}

bool H900userconfig::readActivityHardButtons(pugi::xml_node &buttons)
{
  auto ret = true;
  for (pugi::xml_node prop : buttons.children("Button")) {
    ret &= readActivityHardButton(prop);
  }
  return ret;
}

bool H900userconfig::readActivityHardButton(pugi::xml_node &button)
{
  int buttonPos;
  auto t = item::ButtonType::Hard;
  auto activityPos = c->getActivities().size() - 1;

  auto ret = worker->addActivityButtonCommand(activityPos, t, -1); //append
  if (!ret) {
    return false;
  }

  buttonPos = c->getActivities()[activityPos].getHardButtons().size() - 1;

  auto actionId = string(button.child("ActionId").text().as_string());
  if (actionId.empty()) {
    return false;
  }
  auto action = item::Button::getAction(actionId);
  worker->setActivityButtonAction(action, activityPos, t, buttonPos);
  auto device = item::Button::getDevice(actionId);
  worker->setActivityButtonDevice(device, activityPos, t, buttonPos);

  auto name = button.attribute("name").value();
  worker->setActivityButtonName(string(name), activityPos, t, buttonPos);
  return true;
}

bool H900userconfig::readActivitySoftButtons(pugi::xml_node &buttons)
{
  auto ret = true;
  std::map<int, pugi::xml_node> buttonsByPos;
  int maxPos = -1;
  auto activityPos = c->getActivities().size() - 1;

  for (pugi::xml_node button : buttons.children("Button")) {
    auto uiPos = button.child("Position").text().as_int(-1);
    if (uiPos < 0) {
      return false;
    }
    buttonsByPos[uiPos] = button;
    if (uiPos > maxPos) {
      maxPos = uiPos;
    }
  }

  for (int i = 0; i <= maxPos; i++) {
    auto it = buttonsByPos.find(i);
    if (it == buttonsByPos.end()) {
      ret &= addDummyActivitySoftButton(activityPos, i);
    } else {
      ret &= readActivitySoftButton(it->second, activityPos, i);
    }
  }
  return ret;
}

bool H900userconfig::readActivitySoftButton(pugi::xml_node &button,
    int activityPos, int uiPos)
{
  auto t = item::ButtonType::Soft;

  auto ret = worker->addActivityButtonCommand(activityPos, t, uiPos);
  if (!ret) {
    return false;
  }
  auto actionId = string(button.child("ActionId").text().as_string());
  if (actionId.empty()) {
    return false;
  }
  auto action = item::Button::getAction(actionId);
  worker->setActivityButtonAction(action, activityPos, t, uiPos);
  auto device = item::Button::getDevice(actionId);
  worker->setActivityButtonDevice(device, activityPos, t, uiPos);
  auto label = button.child("Label").child_value();
  worker->setActivityButtonName(string(label), activityPos, t, uiPos);
  worker->setActivityButtonPosition(uiPos, activityPos, t, uiPos);
  auto file = button.child("Icon");
  if (file) {
    auto c = file.text().as_string();
    worker->setActivityButtonFile(string(c), activityPos, t, uiPos);
  }
  return true;
}

bool H900userconfig::addDummyActivitySoftButton(int activityPos, int pos)
{
  auto t = item::ButtonType::Soft;

  auto ret = worker->addActivityButtonCommand(activityPos, t, pos);
  if (!ret) {
    return false;
  }
  worker->setActivityButtonAction(string(item::Button::UNUSED), activityPos, t,
      pos);
  worker->setActivityButtonName("", activityPos, t, pos);
  worker->setActivityButtonPosition(pos, activityPos, t, pos);
  return true;
}

bool H900userconfig::readActivityAction(pugi::xml_node &actions,
    item::ActivityAction t)
{
  auto activityPos = c->getActivities().size() - 1;

  auto ret = worker->addActivityActionCommand(activityPos, t, -1); //append
  if (!ret) {
    return false;
  }
  return readActivityActionSequences(actions, t);
}

bool H900userconfig::readActivityActionSequences(pugi::xml_node &action,
    item::ActivityAction t)
{
  auto ret = true;

  for (pugi::xml_node prop : action.children("Action")) {
    ret &= readActivityActionSequence(prop, t);
  }
  return ret;
}

bool H900userconfig::readActivityActionSequence(pugi::xml_node &sequence,
    item::ActivityAction t)
{
  uint32_t actionPos;
  uint32_t seqPos;

  auto activityPos = c->getActivities().size() - 1;
  switch (t) {
    case item::ActivityAction::Enter:
      actionPos = c->getActivities()[activityPos].getEnterActions().size() - 1;
      seqPos =
          c->getActivities()[activityPos].getEnterActions()[actionPos].sequence.size(); //before append -- not "-1"
      break;
    case item::ActivityAction::Leave:
      actionPos = c->getActivities()[activityPos].getLeaveActions().size() - 1;
      seqPos =
          c->getActivities()[activityPos].getLeaveActions()[actionPos].sequence.size(); //before append -- not "-1"
      break;
    default:
      return false;
  }
  auto ret = worker->addActivityActionSequenceCommand(activityPos, t, actionPos,
      -1); //append
  if (!ret) {
    return false;
  }

  return readActivityActionSequenceData(sequence, activityPos, actionPos, t,
      seqPos);
}

bool H900userconfig::readActivityActionSequenceData(pugi::xml_node &sequence,
    uint32_t activityPos, uint32_t actionPos, item::ActivityAction t,
    uint32_t seqPos)
{
  auto target = sequence.child("Target").text().as_string();
  if (string(target) != "Device") {
    emit writeLog(LogLevel::Warning,
        tr("xml: Action Target != Device in %1. This is not supported").arg(
            qstr(c->getDevices().back().label.get())), ContentType::PlainText);
    return false;
  }
  auto operation = sequence.child("Operation");
  auto opcode = operation.child("Name").text().as_string();
  worker->setActivityActionSequenceOp(Enum<Operation>(opcode), activityPos, t,
      actionPos, seqPos);
  for (pugi::xml_node prop : operation.children("Parameter")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "DeviceId"_hash: {
        auto val = prop.text().as_uint();
        worker->setActivityActionSequenceDeviceId(val, activityPos, t,
            actionPos, seqPos);
        break;
      }
      case "Command"_hash: {
        auto val = prop.text().as_string();
        worker->setActivityActionSequenceCmd(val, activityPos, t, actionPos,
            seqPos);
        break;
      }
      case "Modifier"_hash: {
        auto val = prop.text().as_string();
        worker->setActivityActionSequenceMod(Enum<Modifier>(val), activityPos,
            t, actionPos, seqPos);
        break;
      }
      case "DelayValue"_hash: {
        auto val = prop.text().as_uint();
        worker->setActivityActionSequenceDelayMs(val, activityPos, t, actionPos,
            seqPos);
        break;
      }
      case "State"_hash:
      case "StateName"_hash: {
        auto val = prop.text().as_string();
        worker->setActivityActionSequenceStateName(
            Enum<StateMachineDeviceType>(val), activityPos, t, actionPos,
            seqPos);
        break;
      }
      case "Value"_hash: {
        auto val = prop.text().as_string();
        worker->setActivityActionSequenceValue(val, activityPos, t, actionPos,
            seqPos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import activity action: unknown property (name = %1, "
                "value = %2)").arg(qstr(name)).arg(qstr(unknown.text)),
            ContentType::PlainText);
        worker->setActivityActionUnknownParam(unknown, activityPos, t,
            actionPos, seqPos);
        break;
      }
    }
  }
  return true;
}

bool H900userconfig::readActivityRoles(pugi::xml_node &activity)
{
  auto ret = true;
  for (pugi::xml_node prop : activity.children("Role")) {
    ret &= readActivityRole(prop);
  }
  return ret;
}

bool H900userconfig::readActivityRole(pugi::xml_node &role)
{
  item::Role tmpRole;
  auto activityPos = c->getActivities().size() - 1;

  auto pres = role.child("Presentation").text().as_string("");
  if (!string(pres).empty()) {
    emit writeLog(LogLevel::Warning,
        tr("xml: Role/Presentation non-empty (is %1). This is not supported").arg(
            QString(pres)), ContentType::PlainText);
    return false;
  }

  auto deviceId = role.child("DeviceId").text().as_uint();
  tmpRole.deviceId.set(deviceId);
  auto name = role.child("Name").text().as_string();
  tmpRole.role = Enum<DeviceRole>(name);

  worker->setActivityRoleCommand(activityPos, tmpRole, -1);
  return true;
}

bool H900userconfig::readActivityPowerStateDevices(pugi::xml_node &power)
{
  auto ret = true;
  auto activityPos = c->getActivities().size() - 1;

  for (pugi::xml_node id : power.children("On")) {
    auto deviceId = id.text().as_int();
    if (deviceId == 0) {
      return false;
    }
    ret &= worker->addActivityPowerOnDevicesCommand(activityPos, deviceId, -1);
  }
  for (pugi::xml_node id : power.children("Off")) {
    auto deviceId = id.text().as_int();
    if (deviceId == 0) {
      return false;
    }
    ret &= worker->addActivityPowerOffDevicesCommand(activityPos, deviceId, -1);
  }
  return ret;
}

item::UnknownElement H900userconfig::toUnknownElement(
    const pugi::xml_node &node)
{
  string text;
  map<string, string> attrs;
  vector<item::UnknownElement> children;

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

  return item::UnknownElement(node.name() ? node.name() : "", attrs, text,
      children);
}

void H900userconfig::addId(uint32_t id)
{
  lib::UidGenerator::getInstance().markUsed(id);
}

bool H900userconfig::readIrProto()
{
  binary::irProto::File protocols;
  auto status = protocols.parse(QString(wp + "/" + irProtoPath).toStdString());
  if (status != binary::irProto::Status::OK) {
    emit writeLog(LogLevel::Error,
        tr("import ir protocols parser error: %d").arg((int) status),
        ContentType::PlainText);
    return false;
  }
  auto ret = worker->setIrProtoLib(protocols);
  emit writeLog(LogLevel::Debug,
      tr("read %1 binary ir protocols").arg(
          c->getProtocolLib().getProtocolCount()), ContentType::PlainText);
  return ret;
}

bool H900userconfig::readIrStream()
{
  auto status = streams.parse(QString(wp + "/" + ssIrPath).toStdString());
  if (status != binary::ssIr::Status::OK) {
    emit writeLog(LogLevel::Error,
        tr("import ir stream parser error: %d").arg((int) status),
        ContentType::PlainText);
    return false;
  }
  emit writeLog(LogLevel::Debug,
      tr("read %1 binary ir streams").arg(streams.getStreamCount()),
      ContentType::PlainText);
  return true;
}

bool H900userconfig::dumpUserConfigXml(int pugiFormat)
{
  pugi::xml_document xml;

  auto decl = xml.prepend_child(pugi::node_declaration);
  decl.append_attribute("version").set_value("1.0");
  decl.append_attribute("encoding").set_value("UTF-8");

  auto root = xml.append_child("Root");
  auto ret = writeProperties(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("exporting xml failed (header)"),
        ContentType::PlainText);
    return false;
  }
  ret = writeUser(root);
  ret &= writeController(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("exporting xml failed (user)"),
        ContentType::PlainText);
    return false;
  }
  ret = writeDevices(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("exporting xml failed (devices)"),
        ContentType::PlainText);
    return false;
  }
  ret &= writeActivities(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("exporting xml failed (activities)"),
        ContentType::PlainText);
    return false;
  }
  ret &= writeProtocols(root);
  if (ret != true) {
    emit writeLog(LogLevel::Error, tr("exporting xml failed (protocols)"),
        ContentType::PlainText);
    return false;
  }

#ifdef _WIN32
  ret = xml.save_file(QString(wp + "/" + userConfigPath).toStdWString().c_str(),
      PUGIXML_TEXT("  "), pugiFormat, pugi::encoding_utf8);
#else
  ret = xml.save_file(QString(wp + "/" + userConfigPath).toUtf8(),
      PUGIXML_TEXT("  "), pugiFormat, pugi::encoding_utf8);
#endif
  return true;
}

bool H900userconfig::writeProperties(pugi::xml_node &root)
{
  auto properties = root.append_child("Properties");
  auto property = properties.append_child("Property");
  property.append_attribute("name").set_value("version");
  property.text().set("1.0");
  property = properties.append_child("Property");
  property.append_attribute("name").set_value("ProtocolCacheHash");
  property.text().set(hash);
  property = properties.append_child("Property");
  property.append_attribute("name").set_value("LastUpdated");
  property.text().set(writerTime);
  return true;
}

bool H900userconfig::writeUser(pugi::xml_node &root)
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

bool H900userconfig::writeController(pugi::xml_node &root)
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

bool H900userconfig::writeDevices(pugi::xml_node &root)
{
  bool ret = true;

  const auto &devices = c->getDevices();
  for (const auto &data : devices) {
    auto device = root.append_child("Device");
    ret &= writeDevice(device, data);
  }
  return ret;
}

bool H900userconfig::writeDevice(pugi::xml_node &device,
    const item::Device &data)
{
  bool ret = true;

  // @formatter:off
  device.append_child("Id").text().set(data.getId());
  device.append_child("Type").text().set(data.type.get().getString());
  device.append_child("Manufacturer").text().set(data.mnf.get());
  device.append_child("Model").text().set(data.model.get());

  auto presentation = device.append_child("Presentation");
  presentation.append_child("Label").text().set(data.label.get());
  if (data.getSoftButtons().size() > 0) {
    auto softButtons = presentation.append_child("ControlGroup");
    softButtons.append_attribute("name").set_value("Misc");
    ret &= writeDeviceButtons(softButtons, data.getId(), data.getSoftButtons(),
        item::ButtonType::Soft);
  }
  if (data.getHardButtons().size() > 0) {
    auto hardButtons = presentation.append_child("ControlGroup");
    hardButtons.append_attribute("name").set_value("HardButtons");
    ret &= writeDeviceButtons(hardButtons, data.getId(), data.getHardButtons(),
        item::ButtonType::Hard);
  }

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
  writeProperty(properties, "RecordMedia Fixed Disc",data.recordMediaFixedDisc);
  writeProperty(properties, "RecordMedia Removable Videotape", data.recordMediaRemovableVideotape);
  writeProperty(properties, "RevertInput", data.revertInput);
  writeProperty(properties, "Scart", data.scart);
  writeProperty(properties, "TunerInput", data.tunerInput);
  writeProperty(properties, "VideoSwitch", data.videoSwitch);
  for (const auto &prop : data.getUnknownProperties()) {
    writeUnknownElement(properties, prop);
  }

  if (!data.getStateMachines().empty()) {
    auto states = device.append_child("States");
    ret &= writeStatemachines(states, data.getId(), data.getStateMachines());
  }
  if (data.getNumpad().has_value()) {
    auto numeric = device.append_child("Numeric");
    ret &= writeNumeric(numeric, data.getId(), data.getNumpad().value());
  }
  auto commands = device.append_child("Commands");
  ret &= writeIrList(commands, data.getIrCommands());
// @formatter:on
  return ret;
}

bool H900userconfig::writeDeviceButtons(pugi::xml_node &buttons,
    uint32_t deviceId, const vector<item::Button> &data,
    enum item::ButtonType t)
{
  bool ret = true;

  for (const auto &d : data) {
    if (d.action.get() == item::Button::UNUSED) {
      //marker for skipping export
      continue;
    }
    auto button = buttons.append_child("Button");
    ret &= writeDeviceButton(button, deviceId, d, t);
  }
  return ret;
}

bool H900userconfig::writeDeviceButton(pugi::xml_node &button,
    uint32_t deviceId, const item::Button &data, enum item::ButtonType t)
{
  if (t == item::ButtonType::Hard) {
    button.append_attribute("name").set_value(data.name.get());
    button.append_child("Label"); //empty
  } else if (!data.name.get().empty()) {
    button.append_child("Label").text().set(data.name.get());
  } else {
    button.append_child("Label"); //empty
  }
  if (data.file.isIncluded() == Used::YES) {
    button.append_child("Icon").text().set(data.file.get());
  }
  if (data.position.isIncluded() == Used::YES) {
    button.append_child("Position").text().set(data.position.get());
  }
  button.append_child("ActionId").text().set(data.getActionId(deviceId));
  return true;
}

bool H900userconfig::writeStatemachines(pugi::xml_node &states,
    uint32_t deviceId, const vector<item::StateMachine> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    auto state = states.append_child("State");
    ret &= writeStatemachine(state, deviceId, d);
  }
  return ret;
}

bool H900userconfig::writeStatemachine(pugi::xml_node &state, uint32_t deviceId,
    const item::StateMachine &data)
{
  bool ret = true;

  state.append_child("Id").text().set(data.smType.get().getString());
  if (!data.discrete.empty()) {
    for (const auto &d : data.discrete.states) {
      state.append_child("Value").text().set(d);
    }
  }
  if (!data.relative.empty()) {
    for (const auto &d : data.relative.states) {
      state.append_child("Value").text().set(d);
    }
  }
  if (data.delayMs.isIncluded() == Used::YES) {
    state.append_child("Delay").text().set(data.delayMs.get());
  }

  if (data.startAction.has_value()) {
    auto actionType = state.append_child(
        data.startAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.startAction,
        item::StateMachineType::Unknown);
  }
  if (data.finishAction.has_value()) {
    auto actionType = state.append_child(
        data.finishAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.finishAction,
        item::StateMachineType::Unknown);
  }
  if (!data.discrete.empty()) {
    auto discreteAction = state.append_child("DiscreteActions");
    ret &= writeDiscreteActions(discreteAction, deviceId, data.discrete);
  }
  if (!data.relative.empty()) {
    auto relativeAction = state.append_child("RelativeActions");
    ret &= writeRelativeActions(relativeAction, deviceId, data.relative);
  }
  return ret;
}

bool H900userconfig::writeDiscreteActions(pugi::xml_node &action,
    uint32_t deviceId, const item::DiscreteActions &data)
{
  bool ret = true;

  for (int i = 0; i < data.states.size(); i++) {
    auto actionType = action.append_child(
        data.enterStateAction[i].actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, data.enterStateAction[i],
        item::StateMachineType::Discrete);
    actionType.append_child("Name").text().set(data.states[i]);
  }
  return ret;
}

bool H900userconfig::writeRelativeActions(pugi::xml_node &action,
    uint32_t deviceId, const item::RelativeActions &data)
{
  bool ret = true;

  if (data.resetAction.has_value()) {
    auto actionType = action.append_child(
        data.resetAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.resetAction,
        item::StateMachineType::Relative);
  }
  if (data.nextStateAction.has_value()) {
    auto actionType = action.append_child(
        data.nextStateAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.nextStateAction,
        item::StateMachineType::Relative);
  }
  if (data.prevStateAction.has_value()) {
    auto actionType = action.append_child(
        data.prevStateAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.prevStateAction,
        item::StateMachineType::Relative);
  }
  return ret;
}

bool H900userconfig::writeDeviceAction(pugi::xml_node &actionType,
    uint32_t deviceId, const item::DeviceAction &data, item::StateMachineType t)
{
  for (const auto &s : data.sequence) {
    auto sequence = actionType.append_child("Action");
    sequence.append_child("Target").text().set("Device"); //static value
    auto operation = sequence.append_child("Operation");
    operation.append_child("Name").text().set(s.opcode.get().getString());
    auto id = operation.append_child("Parameter");
    id.append_attribute("name").set_value("DeviceId");
    id.text().set(deviceId);
    writeProperty(operation, "Command", s.cmd, "Parameter");
    writeProperty(operation, "Modifier", s.mod, "Parameter");
    writeProperty(operation, "DelayValue", s.delayMs, "Parameter");
    if (s.opcode.get().getValue() == Operation::ForceValue) {
      writeProperty(operation, "StateName", s.stateName, "Parameter");
    } else {
      writeProperty(operation, "State", s.stateName, "Parameter");
    }
    if (s.value.isIncluded() == Used::YES) {
      writeProperty(operation, "Value", s.value, "Parameter");
    }
    for (const auto &prop : s.getUnknownParams()) {
      writeUnknownElement(operation, prop);
    }
  }
  return true;
}

bool H900userconfig::writeNumeric(pugi::xml_node &numeric, uint32_t deviceId,
    const item::Numpad &data)
{
  bool ret = true;

  if (data.fixedDigits.isIncluded() == Used::YES) {
    numeric.append_child("FixedDigits").text().set(data.fixedDigits.get());
  }

  if (data.start.has_value()) {
    auto actionType = numeric.append_child("Start");
    ret &= writeDeviceAction(actionType, deviceId, *data.start,
        item::StateMachineType::Discrete); //dummy
  }
  if (data.greaterTen.has_value()) {
    auto actionType = numeric.append_child("GreaterTen");
    ret &= writeDeviceAction(actionType, deviceId, *data.greaterTen,
        item::StateMachineType::Discrete); //dummy
  }
  if (data.greaterHundred.has_value()) {
    auto actionType = numeric.append_child("GreaterHundred");
    ret &= writeDeviceAction(actionType, deviceId, *data.greaterHundred,
        item::StateMachineType::Discrete); //dummy
  }
  if (data.finish.has_value()) {
    auto actionType = numeric.append_child("Finish");
    ret &= writeDeviceAction(actionType, deviceId, *data.finish,
        item::StateMachineType::Discrete); //dummy
  }
  if (data.first.has_value()) {
    auto actionType = numeric.append_child("FirstDigit");
    ret &= writeNumericActions(actionType, deviceId, *data.first);
  }
  if (data.middle.has_value()) {
    auto actionType = numeric.append_child("MiddleDigit");
    ret &= writeNumericActions(actionType, deviceId, *data.middle);
  }
  if (data.last.has_value()) {
    auto actionType = numeric.append_child("LastDigit");
    ret &= writeNumericActions(actionType, deviceId, *data.last);
  }
  return true;
}

bool H900userconfig::writeNumericActions(pugi::xml_node &action,
    uint32_t deviceId, const item::Digits &data)
{
  bool ret = true;

  for (int i = 0; i < data.size(); i++) {
    auto actionType = action.append_child("Digit");
    actionType.append_attribute("value").set_value(to_string(i));
    ret &= writeDeviceAction(actionType, deviceId, data[i],
        item::StateMachineType::Discrete); //dummy
  }
  return ret;
}

bool H900userconfig::writeIrList(pugi::xml_node &commands,
    const item::Commands &data)
{
  bool ret = true;

  auto properties = commands.append_child("Properties");
  writeProperty(properties, "PressPreSilence", data.pressPreSilenceMs);
  writeProperty(properties, "PressInterKey", data.pressInterKeyMs);
  writeProperty(properties, "HoldPreSilence", data.holdPreSilenceMs);
  writeProperty(properties, "HoldInterKey", data.holdInterKeyMs);
  for (const auto &prop : data.getUnknownProperties()) {
    writeUnknownElement(properties, prop);
  }

  for (int i = 0; i < data.getRawCommands().size(); i++) {
    auto &cmdData = data.getRawCommands()[i];
    if (cmdData.name.get().empty()) {
      //skip
      continue;
    }
    auto command = commands.append_child("Command");
    ret &= writeIr(command, cmdData);
  }
  for (int i = 0; i < data.getProtoCommands().size(); i++) {
    auto &cmdData = data.getProtoCommands()[i];
    if (cmdData.name.get().empty()
        || (cmdData.codeType.get().getValue() == CodeType::None)
        || (cmdData.codeType.get().getValue() == CodeType::Unknown)) {
      //skip
      continue;
    }
    auto command = commands.append_child("Command");
    ret &= writeIr(command, cmdData);
  }
  return ret;
}

bool H900userconfig::writeIr(pugi::xml_node &command,
    const item::RawCommand &data)
{
  int index;
  vector<uint8_t> raw { 0xff, 0xff }; //start with 0xffff

  command.append_child("Name").text().set(data.name.get());
  auto cmdData = command.append_child("Data");
  cmdData.append_child("Protocol").text().set(-1);

  streams.appendStream(data.stream, index);
  lib::setHarmony16_file(index, raw);
  cmdData.append_child("Code").text().set(lib::bytesToHexString(raw));
  return true;
}

bool H900userconfig::writeIr(pugi::xml_node &command,
    const item::ProtoCommand &data)
{
  if (c->getProtocolLib().getProtocolCount() <= data.protocolIndex.get()) {
    emit writeLog(LogLevel::Error, tr("exporting xml failed (protocol at index"
        "%1 missing)").arg(data.protocolIndex.get()), ContentType::PlainText);
    return false;
  }

  command.append_child("Name").text().set(data.name.get());
  auto cmdData = command.append_child("Data");
  cmdData.append_child("Protocol").text().set(data.protocolIndex.get());
  if (data.canDecode.get() == true) {
    auto raw = data.command.serialiseStr();
    cmdData.append_child("Code").text().set(raw);
  } else {
    const auto &raw = data.data.get();
    cmdData.append_child("Code").text().set(lib::bytesToHexString(raw));
  }
  return true;
}

bool H900userconfig::writeActivities(pugi::xml_node &root)
{
  bool ret = true;

  auto powerOffActivity = root.append_child("Activity");
  ret &= writePowerOffActivity(powerOffActivity);

  const auto &activities = c->getActivities();
  for (const auto &data : activities) {
    auto activity = root.append_child("Activity");
    ret &= writeActivity(activity, data);
  }
  return ret;
}

bool H900userconfig::writePowerOffActivity(pugi::xml_node &activity)
{
  activity.append_child("Id").text().set(-1); //magic for power off
  activity.append_child("Type").text().set("PowerOff");
  auto presentation = activity.append_child("Presentation");
  presentation.append_child("Label").text().set("PowerOff");

  auto enterAction = activity.append_child("EnterActions");
  for (const auto &device : c->getDevices()) {
    if ((device.manualPower.get() == true) || (device.alwaysOn.get() == true)) {
      //don't power off
      continue;
    }
    for (const auto &stateMachine : device.getStateMachines()) {
      if (stateMachine.smType.get().getValue()
          == StateMachineDeviceType::Power) {
        auto sequence = enterAction.append_child("Action");
        sequence.append_child("Target").text().set("Device"); //static value
        auto operation = sequence.append_child("Operation");
        operation.append_child("Name").text().set(
            Enum<Operation>(Operation::SetValue).getString());
        auto param = operation.append_child("Parameter");
        param.append_attribute("name").set_value("DeviceId");
        param.text().set(device.getId());
        param = operation.append_child("Parameter");
        param.append_attribute("name").set_value("State");
        param.text().set(
            Enum<StateMachineDeviceType>(StateMachineDeviceType::Power).getString());
        param = operation.append_child("Parameter");
        param.append_attribute("name").set_value("Value");
        param.text().set("Off");

        //found
        break;
      }
    }
  }

  //all, ignoring "manual power" / "always on"
  auto power = activity.append_child("Power");
  for (const auto &offDeviceId : c->getDeviceIds()) {
    power.append_child("Off").text().set(offDeviceId);
  }
  return true;
}

bool H900userconfig::writeActivity(pugi::xml_node &activity,
    const item::Activity &data)
{
  bool ret = true;

  // @formatter:off
  activity.append_child("Id").text().set(data.getId());
  activity.append_child("Type").text().set(data.type.get().getString());

  auto properties = activity.append_child("Properties");
  writeProperty(properties, "ActivityStartPage", data.pvrType);
  writeProperty(properties, "ControlGroup_Hard Buttons", data.controlGroup_HardButtons);
  writeProperty(properties, "PowerOffUnusedDevices", data.powerOffUnusedDevices);
  writeProperty(properties, "TrainingWheels", data.trainingWheels);
  writeProperty(properties, "UnusedDevicesHelp", data.unusedDevicesHelp);
  writeProperty(properties, "ChannelButtonBehaviour", data.channelButtonBehaviour);
  writeProperty(properties, "ControlGroup_Soft Buttons", data.controlGroup_SoftButtons);
  writeProperty(properties, "EnableSmartMenu", data.enableSmartMenu);
  writeProperty(properties, "EnableSmartZoom", data.enableSmartZoom);
  writeProperty(properties, "GuideButtonMode", data.guideButtonMode);
  writeProperty(properties, "HideModeControl", data.hideModeControl);
  writeProperty(properties, "HideModeListen", data.hideModeListen);
  writeProperty(properties, "HideModeNavigate", data.hideModeNavigate);
  writeProperty(properties, "HideModePlay", data.hideModePlay);
  writeProperty(properties, "HideModePlayMode", data.hideModePlayMode);
  writeProperty(properties, "HideSurfAllChannels", data.hideSurfAllChannels);
  writeProperty(properties, "HideSurfAllShows", data.hideSurfAllShows);
  writeProperty(properties, "HideSurfFavoriteChannels", data.hideSurfFavoriteChannels);
  writeProperty(properties, "HideSurfFavoriteShows", data.hideSurfFavoriteShows);
  writeProperty(properties, "MaxTvContentDays", data.maxTvContentDays);
  writeProperty(properties, "MediaButtonMode", data.mediaButtonMode);
  writeProperty(properties, "PlayOnEnter", data.playOnEnter);
  writeProperty(properties, "RetainStop", data.retainStop);
  writeProperty(properties, "ScrollChannelsByPage", data.scrollChannelsByPage);
  writeProperty(properties, "ScrollShowsByPage", data.scrollShowsByPage);
  writeProperty(properties, "StopOnExit", data.stopOnExit);
  for (const auto &prop : data.getUnknownProperties()) {
    writeUnknownElement(properties, prop);
  }

  auto presentation = activity.append_child("Presentation");
  presentation.append_child("Label").text().set(data.label.get());
  if (!data.getChannels().empty()) {
    auto channelButtons = presentation.append_child("ChannelList");
    ret &= writeActivityChannels(channelButtons, data.getChannels());
  }
  if (data.getSoftButtons().size() > 0) {
    auto softButtons = presentation.append_child("ControlGroup");
    softButtons.append_attribute("name").set_value("Misc");
    ret &= writeActivityButtons(softButtons, data.getSoftButtons(),
        item::ButtonType::Soft);
  }
  if (data.getHardButtons().size() > 0) {
    auto hardButtons = presentation.append_child("ControlGroup");
    hardButtons.append_attribute("name").set_value("HardButtons");
    ret &= writeActivityButtons(hardButtons, data.getHardButtons(),
        item::ButtonType::Hard);
  }
  if (!data.getEnterActions().empty()) {
    auto enterAction = activity.append_child("EnterActions");
    ret &= writeActivityActions(enterAction, data.getEnterActions());
  }
  if (!data.getLeaveActions().empty()) {
    auto leaveAction = activity.append_child("LeaveActions");
    ret &= writeActivityActions(leaveAction, data.getLeaveActions());
  }
  if (!data.getRoles().empty()) {
    ret &= writeActivityRoles(activity, data.getRoles());
  }
  auto power = activity.append_child("Power");
  ret &= writeActivityPowerStateDevices(power, data);
// @formatter:on
  return ret;
}

bool H900userconfig::writeActivityChannels(pugi::xml_node &channels,
    const std::vector<item::Channel> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    auto channel = channels.append_child("Channel");
    ret &= writeActivityChannel(channel, d);
  }
  return ret;
}

bool H900userconfig::writeActivityChannel(pugi::xml_node &channel,
    const item::Channel &data)
{
  channel.append_child("Station").text().set(data.station.get());
  channel.append_child("Number").text().set(data.channel.get());
  channel.append_child("Slot").text().set(data.position.get());
  channel.append_child("Image").text().set(data.img.get());
  return true;
}

bool H900userconfig::writeActivityButtons(pugi::xml_node &buttons,
    const std::vector<item::Button> &data, enum item::ButtonType t)
{
  bool ret = true;

  for (const auto &d : data) {
    if (d.action.get() == item::Button::UNUSED) {
      //marker for skipping export
      continue;
    }
    auto button = buttons.append_child("Button");
    ret &= writeActivityButton(button, d, t);
  }
  return ret;
}

bool H900userconfig::writeActivityButton(pugi::xml_node &button,
    const item::Button &data, enum item::ButtonType t)
{
  if (t == item::ButtonType::Hard) {
    button.append_attribute("name").set_value(data.name.get());
    button.append_child("Label"); //empty
  } else if (!data.name.get().empty()) {
    button.append_child("Label").text().set(data.name.get());
  } else {
    button.append_child("Label"); //empty
  }
  if (data.file.isIncluded() == Used::YES) {
    button.append_child("Icon").text().set(data.file.get());
  }
  if (data.position.isIncluded() == Used::YES) {
    button.append_child("Position").text().set(data.position.get());
  }
  button.append_child("ActionId").text().set(data.getActionId());
  return true;
}

bool H900userconfig::writeActivityActions(pugi::xml_node &action,
    const std::vector<item::DeviceAction> &data)
{
  bool ret = true;

  for (const auto &item : data) {
    ret &= writeActivityAction(action, item);
  }
  return ret;
}

bool H900userconfig::writeActivityAction(pugi::xml_node &actionType,
    const item::DeviceAction &data)
{
  for (const auto &s : data.sequence) {
    auto sequence = actionType.append_child("Action");
    sequence.append_child("Target").text().set("Device"); //static value
    auto operation = sequence.append_child("Operation");
    operation.append_child("Name").text().set(s.opcode.get().getString());
    writeProperty(operation, "DeviceId", s.deviceId, "Parameter");
    writeProperty(operation, "Command", s.cmd, "Parameter");
    writeProperty(operation, "Modifier", s.mod, "Parameter");
    writeProperty(operation, "DelayValue", s.delayMs, "Parameter");
    if (s.opcode.get().getValue() == Operation::ForceValue) {
      writeProperty(operation, "StateName", s.stateName, "Parameter");
    } else {
      writeProperty(operation, "State", s.stateName, "Parameter");
    }
    writeProperty(operation, "Value", s.value, "Parameter");
    for (const auto &prop : s.getUnknownParams()) {
      writeUnknownElement(operation, prop);
    }
  }
  return true;
}

bool H900userconfig::writeActivityRoles(pugi::xml_node &activity,
    const std::vector<item::Role> &data)
{
  bool ret = true;

  for (const auto &item : data) {
    ret &= writeActivityRole(activity, item);
  }
  return ret;
}

bool H900userconfig::writeActivityRole(pugi::xml_node &activity,
    const item::Role &data)
{
  auto role = activity.append_child("Role");
  role.append_child("Name").text().set(data.role.getString());
  role.append_child("DeviceId").text().set(data.deviceId.get());
  role.append_child("Presentation"); //empty
  return true;
}

bool H900userconfig::writeActivityPowerStateDevices(pugi::xml_node &power,
    const item::Activity &data)
{
  for (const auto &onDeviceId : data.getPowerOnDevices()) {
    power.append_child("On").text().set(onDeviceId);
  }
  for (const auto &offDeviceId : data.getPowerOffDevices()) {
    power.append_child("Off").text().set(offDeviceId);
  }
  return true;
}

bool H900userconfig::writeProtocols(pugi::xml_node &root)
{
  bool ret = true;

  auto prot = root.append_child("Protocols");
  for (int i = 0; i < c->getProtocolLib().getProtocolCount(); i++) {
    ret &= writeProtocol(prot, i, c->getProtocolLib().accessProtocol(i));
  }
  prot.append_child("Hash").text().set(hash);
  return ret;
}

bool H900userconfig::writeProtocol(pugi::xml_node &protocols, int idx,
    const binary::irProto::IrProto &irProto)
{
  for (int i = 0; i < irProto.getSectionCount(); i++) {
    if (i > 0) {
      //fixme we have exactly one sample for this.
      //toggle bits are only dumped when in section 0, but ignored in other sections
      //is this a bug in the original h900 software or intentional?
      return true;
    }
    const auto &section = irProto.accessSection(i);
    if (!section.hasToggle()) {
      continue;
    }

    auto prot = protocols.append_child("Protocol");
    prot.append_attribute("index").set_value(idx);
    pugi::xml_node codeSequence = prot.append_child("CodeSequence");
    codeSequence.append_attribute("index").set_value(0);
    codeSequence.append_child("ToggleBit1").text().set(section.getToggle());
    //never seen two toggle bits
  }
  return true;
}

void H900userconfig::writeUnknownElement(pugi::xml_node &parent,
    const item::UnknownElement &element)
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

bool H900userconfig::dumpActionListXml(int pugiFormat)
{
  pugi::xml_document xml;

  auto decl = xml.prepend_child(pugi::node_declaration);
  decl.append_attribute("version").set_value("1.0");
  decl.append_attribute("encoding").set_value("UTF-8");

  auto root = xml.append_child("Root");
  auto ret = exportDevices(root);

  auto properties = root.append_child("Properties");
  auto property = properties.append_child("Property");
  property.append_attribute("name").set_value("ProtocolCacheHash");
  property.text().set(hash);
  property = properties.append_child("Property");
  property.append_attribute("name").set_value("LastUpdated");
  property.text().set(writerTime);
  auto protocols = root.append_child("Protocols");
  protocols.append_child("Hash").text().set(hash);
  if (!ret) {
    return false;
  }

#ifdef _WIN32
  ret = xml.save_file(QString(wp + "/" + actionListPath).toStdWString().c_str(),
      PUGIXML_TEXT("  "), pugiFormat, pugi::encoding_utf8);
#else
  ret = xml.save_file(QString(wp + "/" + actionListPath).toUtf8(),
      PUGIXML_TEXT("  "), pugiFormat, pugi::encoding_utf8);
#endif
  return true;
}

bool H900userconfig::exportDevices(pugi::xml_node &root)
{
  bool ret = true;

  const auto &devices = c->getDevices();
  for (const auto &data : devices) {
    ret &= exportDevice(root, data);
  }
  return ret;
}

bool H900userconfig::exportDevice(pugi::xml_node &root,
    const item::Device &data)
{
  bool ret = true;

  ret &= exportButtons(root, data.getId(), data.getHardButtons());
  ret &= exportButtons(root, data.getId(), data.getSoftButtons());
  return ret;
}

bool H900userconfig::exportButtons(pugi::xml_node &root, uint32_t deviceId,
    const std::vector<item::Button> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    ret &= exportButton(root, deviceId, d);
  }
  return ret;
}

bool H900userconfig::exportButton(pugi::xml_node &root, uint32_t deviceId,
    const item::Button &data)
{
  string str;

  auto actionList = root.append_child("ActionList");
  actionList.append_attribute("name").set_value(data.getActionId(deviceId));
  auto action = actionList.append_child("Action");
  action.append_child("Target").text().set("Device");
  auto op = action.append_child("Operation");
  op.append_child("Name").text().set("SendCommand");

  auto param = op.append_child("Parameter");
  param.append_attribute("name").set_value("DeviceId");
  param.text().set(deviceId);
  param = op.append_child("Parameter");
  param.append_attribute("name").set_value("Command");
  param.text().set(data.action.get());
  param = op.append_child("Parameter");
  param.append_attribute("name").set_value("Modifier");
  param.text().set("Hold");
  return true;
}

bool H900userconfig::writeIrProto()
{
  uint32_t crc;
  vector<uint8_t> tmp;

  auto status = c->getProtocolLib().serialise(
      QString(wp + "/" + irProtoPath).toStdString(), &crc);
  if (status != binary::irProto::Status::OK) {
    emit writeLog(LogLevel::Error,
        tr("write ir protocols parser error: %1").arg((int) status),
        ContentType::PlainText);
    return false;
  }
  lib::setHarmony32_network(crc, tmp);
  hash = lib::bytesToHexString(tmp);
  emit writeLog(LogLevel::Debug,
      tr("wrote %1 binary ir protocols").arg(
          c->getProtocolLib().getProtocolCount()), ContentType::PlainText);
  return true;
}

bool H900userconfig::writeIrStream()
{
  auto status = streams.serialise(QString(wp + "/" + ssIrPath).toStdString());
  if (status != binary::ssIr::Status::OK) {
    emit writeLog(LogLevel::Error,
        tr("write ir stream parser error: %1").arg((int) status),
        ContentType::PlainText);
    return false;
  }
  emit writeLog(LogLevel::Debug,
      tr("wrote %1 binary ir streams").arg(streams.getStreamCount()),
      ContentType::PlainText);
  return true;
}

H900platformconfig::H900platformconfig(const QString &workPath,
    const QString &sourcePath, const QString &defaultsPath) :
    wp(workPath), sp(sourcePath), dp(defaultsPath)
{
}

bool H900platformconfig::dump(const data::ConfigData *c)
{
  QDir sourceDir(sp);
  const QDir workDir(wp);

  if (!sourceDir.exists() || sourceDir.entryList(QDir::Files).isEmpty()) {
    sourceDir = QDir(dp);
  }
  if (!workDir.exists()) {
    return false;
  }

  for (const QString &file : copyFiles) {
    const QString sourcePath = sourceDir.filePath(file);
    const QString destinationPath = workDir.filePath(file);
    QFileInfo sourceInfo(sourcePath);

    if (!sourceInfo.exists()) {
      return false;
    }

    if (QFile::exists(destinationPath)) {
      continue;
    }
    QDir destinationParent(QFileInfo(destinationPath).path());

    if (!destinationParent.exists()) {
      if (!destinationParent.mkpath(".")) {
        return false;
      }
    }

    if (!QFile::copy(sourcePath, destinationPath)) {
      return false;
    }
    QFile destinationFile(destinationPath);
    destinationFile.setPermissions(sourceInfo.permissions());
  }
  return true;
}

bool H900platformconfig::read(const data::ConfigData *c,
    data::CmdCatalogue *worker)
{
  return true;
}

}
}
