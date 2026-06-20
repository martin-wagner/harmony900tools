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
        ret &= readButtons(prop, item::ButtonType::Soft);
        break;
      }
      case "HardButtons"_hash: {
        ret &= readButtons(prop, item::ButtonType::Hard);
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

  if (!ret) {
    return false;
  }

  //todo weitere

  return true;
}

bool ConfigH900::readButtons(pugi::xml_node &buttons, enum item::ButtonType t)
{
  auto ret = true;
  for (pugi::xml_node prop : buttons.children("Button")) {
    ret &= readButton(prop, t);
  }
  return ret;
}

bool ConfigH900::readButton(pugi::xml_node &button, enum item::ButtonType t)
{
  int buttonPos;

  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addButtonCommand(t, devicePos, -1); //append
  if (!ret) {
    return false;
  }
  if (t == item::ButtonType::Hard) {
    buttonPos = c->getDevices()[devicePos].getHardButtons().size() - 1;
  } else {
    buttonPos = c->getDevices()[devicePos].getSoftButtons().size() - 1;
  }

  auto actionId = string(
      button.child("ActionId").text().as_string("1_unknown_Hold"));
  auto action = item::Button::getAction(actionId);
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
  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addStatemachineCommand(devicePos, -1); //append
  if (!ret) {
    return false;
  }
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto id = state.child("Id").text().as_string();
  worker->setStatemachineType(Enum<StateMachineType>(id), devicePos, smPos);
  auto delay = state.child("Delay");
  if (delay) {
    worker->setStatemachineDelay(delay.text().as_uint(0), devicePos, smPos);
  }

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
  ret = readDiscreteActions(discreteActions);
  ret &= readRelativeActions(state);
  return ret;
}

bool ConfigH900::readDiscreteActions(pugi::xml_node &actions)
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

bool ConfigH900::readDiscreteAction(pugi::xml_node &action)
{
  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto name = QString(action.child("Name").text().as_string());
  if (name.isEmpty()) {
    emit writeLog(LogLevel::Error,
        tr("xml: state name is empty in %1").arg(
            QString::fromStdString(c->getDevices().back().label.get())),
        ContentType::PlainText);
    return false;
  }

  auto ret = worker->addStateCommand(devicePos, smPos,
      item::StateTransitionType::Discrete, name, -1); //append
  if (!ret) {
    return false;
  }
  auto actPos =
      c->getDevices()[devicePos].getStateMachines()[smPos].discrete.states.size()
          - 1;
  auto type = Enum<ActionType>(action.name());
  worker->setActionType(type, devicePos, smPos,
      item::StateTransitionAction::Discrete_Enter, actPos);
  return readDiscreteActionSequences(action);
}

bool ConfigH900::readDiscreteActionSequences(pugi::xml_node &action)
{
  auto ret = true;

  for (pugi::xml_node prop : action.children("Action")) {
    ret &= readDiscreteActionSequence(prop);
  }
  return ret;
}

bool ConfigH900::readDiscreteActionSequence(pugi::xml_node &sequence)
{
  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;
  auto actPos =
      c->getDevices()[devicePos].getStateMachines()[smPos].discrete.states.size()
          - 1;
  worker->addActionSequenceCommand(devicePos, smPos, actPos,
      item::StateTransitionAction::Discrete_Enter, -1); //append
  auto seqPos =
      c->getDevices()[devicePos].getStateMachines()[smPos].discrete.enterStateAction[actPos].sequence.size()
          - 1;
  return readActionSequenceData(sequence, devicePos, smPos, actPos,
      item::StateTransitionAction::Discrete_Enter, seqPos);
}

bool ConfigH900::readRelativeActions(pugi::xml_node &state)
{
  auto ret = true;

  if (state.child("RelativeActions").children().empty()) {
    return true;
  }

  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  //relative actions consists of state names and transition actions as separate items.
  //state names
  for (pugi::xml_node prop : state.children("Value")) {
    auto name = prop.text().as_string();
    ret &= worker->addStateCommand(devicePos, smPos,
        item::StateTransitionType::Relative, name, -1); //append
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

bool ConfigH900::readRelativeAction(pugi::xml_node &action)
{
  item::StateTransitionAction add;
  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto type = Enum<ActionType>(action.name());
  switch (type.getValue()) {
    case ActionType::NextAction:
      add = item::StateTransitionAction::Relative_Next;
      break;
    case ActionType::PrevAction:
      add = item::StateTransitionAction::Relative_Prev;
      break;
    case ActionType::ResetAction:
      add = item::StateTransitionAction::Relative_Reset;
      break;
    default:
      emit writeLog(LogLevel::Error,
          tr("xml: action type %1 unknown").arg(type.getQString()),
          ContentType::PlainText);
      return false;
  }
  auto ret = worker->addActionCommand(devicePos, smPos, add);
  if (!ret) {
    return false;
  }
  worker->setActionType(type, devicePos, smPos, add, 0);
  return readRelativeActionSequences(action, add);
}

bool ConfigH900::readRelativeActionSequences(pugi::xml_node &action,
    item::StateTransitionAction t)
{
  auto ret = true;

  for (pugi::xml_node prop : action.children("Action")) {
    ret &= readRelativeActionSequence(prop, t);
  }
  return ret;
}

bool ConfigH900::readRelativeActionSequence(pugi::xml_node &sequence,
    item::StateTransitionAction t)
{
  uint32_t seqPos;

  auto devicePos = c->getDevices().size() - 1;
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  worker->addActionSequenceCommand(devicePos, smPos, 0, t, -1); //append
  switch (t) {
    case item::StateTransitionAction::Relative_Reset:
      seqPos =
          c->getDevices()[devicePos].getStateMachines()[smPos].relative.resetAction->sequence.size()
              - 1;
      break;
    case item::StateTransitionAction::Relative_Next:
      seqPos =
          c->getDevices()[devicePos].getStateMachines()[smPos].relative.nextStateAction->sequence.size()
              - 1;
      break;
    case item::StateTransitionAction::Relative_Prev:
      seqPos =
          c->getDevices()[devicePos].getStateMachines()[smPos].relative.prevStateAction->sequence.size()
              - 1;
      break;
    default:
      return false;
  }
  return readActionSequenceData(sequence, devicePos, smPos, 0, t, seqPos);
}

bool ConfigH900::readActionSequenceData(pugi::xml_node &sequence,
    uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    item::StateTransitionAction t, uint32_t seqPos)
{
  auto target = sequence.child("Target").text().as_string();
  if (string(target) != "Device") {
    emit writeLog(LogLevel::Warning,
        tr("xml: Action Target != Device in %1. This is not supported").arg(
            QString::fromStdString(c->getDevices().back().label.get())),
        ContentType::PlainText);
    return false;
  }
  auto operation = sequence.child("Operation");
  auto opcode = operation.child("Name").text().as_string();
  worker->setActionSequenceOp(Enum<Operation>(opcode), devicePos, smPos,
      t, actPos, seqPos);
  for (pugi::xml_node prop : operation.children("Parameter")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "DeviceId"_hash:
        //don't store redundant data
        break;
      case "Command"_hash: {
        auto val = prop.text().as_string();
        worker->setActionSequenceCmd(val, devicePos, smPos, t, actPos,
            seqPos);
        break;
      }
      case "Modifier"_hash: {
        auto val = prop.text().as_string();
        worker->setActionSequenceMod(Enum<Modifier>(val), devicePos,
            smPos, t, actPos, seqPos);
        break;
      }
      case "DelayValue"_hash: {
        auto val = prop.text().as_uint();
        worker->setActionSequenceDelayMs(val, devicePos, smPos, t,
            actPos, seqPos);
        break;
      }
      case "State"_hash:
      case "StateName"_hash: {
        auto val = prop.text().as_string();
        worker->setActionSequenceStateName(Enum<StateMachineType>(val),
            devicePos, smPos, t, actPos, seqPos);
        break;
      }
      case "Value"_hash: {
        auto val = prop.text().as_string();
        worker->setActionSequenceStateValue(val, devicePos, smPos, t,
            actPos, seqPos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import device action: unknown property (value = %1)").arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
        worker->setActionUnknownParam(unknown, devicePos, smPos, t, actPos,
            seqPos);
        break;
      }
    }
  }
  return true;
}

bool ConfigH900::readNumeric(pugi::xml_node &numeric)
{
  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addNumpadCommand(devicePos);
  if (!ret) {
    return false;
  }
  auto digits = numeric.child("FixedDigits");
  if (digits) {
    worker->setNumpadFixedDigits(digits.text().as_uint(), devicePos);
  }

  ret = true;
  auto firstDigit = numeric.child("FirstDigit");
  if (firstDigit) {
    ret &= worker->addNumpadDigitsCommand(devicePos, item::DigitSection::First);
    //todo read data
  }
  auto middleDigit = numeric.child("MiddleDigit");
  if (middleDigit) {
    ret &= worker->addNumpadDigitsCommand(devicePos, item::DigitSection::Middle);
    //todo read data
  }
  auto lastDigit = numeric.child("LastDigit");
  if (lastDigit) {
    ret &= worker->addNumpadDigitsCommand(devicePos, item::DigitSection::Last);
    //todo read data
  }
  auto finish = numeric.child("Finish");
  if (finish) {
    ret &= worker->addNumpadDigitsCommand(devicePos, item::DigitSection::Finish);
    //todo read data
  }
  return ret;
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

item::UnknownElement ConfigH900::toUnknownElement(const pugi::xml_node &node)
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

bool ConfigH900::writeDevice(pugi::xml_node &device, const item::Device &data)
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
  if (data.getNumpad() != nullopt) {
    auto numeric = device.append_child("Numeric");
    ret &= writeNumeric(numeric, data.getNumpad().value());
  }

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
    const vector<item::Button> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    auto button = buttons.append_child("Button");
    ret &= writeButton(button, deviceId, d);
  }
  return ret;
}

bool ConfigH900::writeButton(pugi::xml_node &button, uint32_t deviceId,
    const item::Button &data)
{
  if (data.getButtonType() == item::ButtonType::Hard) {
    button.append_attribute("name").set_value(data.name.get());
    button.append_child("Label"); //empty
  } else {
    button.append_child("Label").text().set(data.name.get());
  }
  if (data.position.isIncluded() == Include::ALWAYS) {
    button.append_child("Position").text().set(data.position.get());
  }
  button.append_child("ActionId").text().set(data.getActionId(deviceId));
  return true;
}

bool ConfigH900::writeStatemachines(pugi::xml_node &states, uint32_t deviceId,
    const vector<item::StateMachine> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    auto state = states.append_child("State");
    ret &= writeStatemachine(state, deviceId, d);
  }
  return ret;
}

bool ConfigH900::writeStatemachine(pugi::xml_node &state, uint32_t deviceId,
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
  if (data.delayMs.isIncluded() == Include::ALWAYS) {
    state.append_child("Delay").text().set(data.delayMs.get());
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

bool ConfigH900::writeDiscreteActions(pugi::xml_node &action, uint32_t deviceId,
    const item::DiscreteActions &data)
{
  bool ret = true;

  for (int i = 0; i < data.states.size(); i++) {
    auto actionType = action.append_child(
        data.enterStateAction[i].actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, data.enterStateAction[i],
        item::StateTransitionType::Discrete);
    actionType.append_child("Name").text().set(data.states[i]);
  }
  return ret;
}

bool ConfigH900::writeRelativeActions(pugi::xml_node &action, uint32_t deviceId,
    const item::RelativeActions &data)
{
  bool ret = true;

  if (data.resetAction != nullopt) {
    auto actionType = action.append_child(
        data.resetAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.resetAction,
        item::StateTransitionType::Relative);
  }
  if (data.nextStateAction != nullopt) {
    auto actionType = action.append_child(
        data.nextStateAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.nextStateAction,
        item::StateTransitionType::Relative);
  }
  if (data.prevStateAction != nullopt) {
    auto actionType = action.append_child(
        data.prevStateAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.prevStateAction,
        item::StateTransitionType::Relative);
  }
  return ret;
}

bool ConfigH900::writeDeviceAction(pugi::xml_node &actionType,
    uint32_t deviceId, const item::DeviceAction &data,
    item::StateTransitionType t)
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
    if (t != item::StateTransitionType::Relative)
      writeProperty(operation, "State", s.stateName, "Parameter");
    else {
      //fixme we have exactly one sample for this. no idea if this is correct.
      writeProperty(operation, "StateName", s.stateName, "Parameter");
    }
    if (s.stateValue.isIncluded() == Include::ALWAYS) {
      writeProperty(operation, "Value", s.stateValue, "Parameter");
    }
    for (const auto &prop : s.getUnknownParams()) {
      writeUnknownElement(operation, prop);
    }
  }
  return true;
}

bool ConfigH900::writeNumeric(pugi::xml_node &numeric, const item::Numpad &data)
{
  if (data.fixedDigits.isIncluded() == Include::ALWAYS) {
    numeric.append_child("FixedDigits").text().set(data.fixedDigits.get());
  }
  if (data.finish != nullopt) {
    numeric.append_child("Finish");
    //todo content
  }
  if (data.first != nullopt) {
    numeric.append_child("FirstDigit");
    //todo content
  }
  if (data.middle != nullopt) {
    numeric.append_child("MiddleDigit");
    //todo content
  }
  if (data.last != nullopt) {
    numeric.append_child("LastDigit");
    //todo content
  }
  return true;
}

void ConfigH900::writeUnknownElement(pugi::xml_node &parent,
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

}
}
