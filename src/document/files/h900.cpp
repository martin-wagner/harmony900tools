// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <pugixml.hpp>

#include "version.h"
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
  vector<uint8_t> tmp;

  this->c = c;
  this->worker = nullptr;
  writerTime = lib::writeTimeH900Xml();

  emit writeLog(LogLevel::Info, tr("Exporting data..."),
      ContentType::PlainText);

  try {
    ret &= writeIrProto(); //side effect: creates hash for xml files
    ret &= writeIrStream();
    ret &= dumpUserConfigXml();
    ret &= dumpActionListXml();
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

bool ConfigH900::read(const ConfigData *c, CmdCatalogue *worker)
{
  bool ret = true;

  this->c = c;
  this->worker = worker;

  emit writeLog(LogLevel::Info, tr("Importing user config..."),
      ContentType::PlainText);

  try {
    ret &= readUserConfigXml();
    ret &= readIrProto();
    ret &= readIrStream();
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
  if (ret) {
    emit writeLog(LogLevel::Debug,
        tr("Importing xml successful, contains %1 devices and %2 activities").arg(
            c->getDevices().size()).arg(c->getActivities().size()),
        ContentType::PlainText);
  } else {
    emit writeLog(LogLevel::Error, tr("Importing xml failed"),
        ContentType::PlainText);
  }
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

bool ConfigH900::readUser(pugi::xml_node &root)
{
  auto user = root.child("User");

  auto id = user.child("Id").text().as_uint();
  addId(id);
  worker->setUserId(id);
  auto firstName = user.child("Presentation").child("FirstName").child_value();
  worker->setUserFirstName(QString::fromStdString(firstName));
  auto lastName = user.child("Presentation").child("LastName").child_value();
  worker->setUserLastName(QString::fromStdString(lastName));
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
                QString::fromStdString(name)).arg(
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

  emit writeLog(LogLevel::Info, tr("Reading devices..."),
      ContentType::PlainText);

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
                QString::fromStdString(name)).arg(
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
        ret &= readDeviceButtons(prop, item::ButtonType::Soft);
        break;
      }
      case "HardButtons"_hash: {
        ret &= readDeviceButtons(prop, item::ButtonType::Hard);
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

bool ConfigH900::readDeviceButtons(pugi::xml_node &buttons,
    enum item::ButtonType t)
{
  auto ret = true;
  for (pugi::xml_node prop : buttons.children("Button")) {
    ret &= readDeviceButton(prop, t);
  }
  return ret;
}

bool ConfigH900::readDeviceButton(pugi::xml_node &button,
    enum item::ButtonType t)
{
  int buttonPos;

  auto devicePos = c->getDevices().size() - 1;

  auto ret = worker->addDeviceButtonCommand(t, devicePos, -1); //append
  if (!ret) {
    return false;
  }
  buttonPos = c->getDevices()[devicePos].getButtons().size() - 1;

  auto actionId = string(
      button.child("ActionId").text().as_string("1_unknown_Hold"));
  auto action = item::Button::getAction(actionId);
  worker->setDeviceButtonAction(action, devicePos, buttonPos);

  //hard/soft are different
  auto name = button.attribute("name");
  if (name) {
    auto c = name.value();
    worker->setDeviceButtonName(string(c), devicePos, buttonPos);
  } else {
    auto c = button.child("Label").child_value();
    worker->setDeviceButtonName(string(c), devicePos, buttonPos);
  }

  //only soft buttons
  auto pos = button.child("Position");
  if (pos) {
    auto p = pos.text().as_uint(0);
    worker->setDeviceButtonPosition(p, devicePos, buttonPos);
  }
  auto file = button.child("Icon");
  if (file) {
    auto c = name.value();
    worker->setDeviceButtonFile(string(c), devicePos, buttonPos);
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

  auto ret = worker->addDeviceStatemachineCommand(devicePos, -1); //append
  if (!ret) {
    return false;
  }
  auto smPos = c->getDevices()[devicePos].getStateMachines().size() - 1;

  auto id = state.child("Id").text().as_string();
  worker->setDeviceStatemachineType(Enum<StateMachineType>(id), devicePos,
      smPos);
  auto delay = state.child("Delay");
  if (delay) {
    worker->setDeviceStatemachineDelay(delay.text().as_uint(0), devicePos,
        smPos);
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

  auto ret = worker->addDeviceSmStateCommand(devicePos, smPos,
      item::StateTransitionType::Discrete, name, -1); //append
  if (!ret) {
    return false;
  }
  auto actPos =
      c->getDevices()[devicePos].getStateMachines()[smPos].discrete.states.size()
          - 1;
  auto type = Enum<ActionType>(action.name());
  worker->setDeviceSmActionType(type, devicePos, smPos,
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
  worker->addDeviceStateActionSequenceCommand(devicePos, smPos, actPos,
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
    ret &= worker->addDeviceSmStateCommand(devicePos, smPos,
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
  auto ret = worker->addDeviceSmActionCommand(devicePos, smPos, add);
  if (!ret) {
    return false;
  }
  worker->setDeviceSmActionType(type, devicePos, smPos, add, 0);
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

  worker->addDeviceStateActionSequenceCommand(devicePos, smPos, 0, t, -1); //append
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
  worker->setDeviceStateActionSequenceOp(Enum<Operation>(opcode), devicePos,
      smPos, t, actPos, seqPos);
  for (pugi::xml_node prop : operation.children("Parameter")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "DeviceId"_hash:
        //don't store redundant data
        break;
      case "Command"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceStateActionSequenceCmd(val, devicePos, smPos, t,
            actPos, seqPos);
        break;
      }
      case "Modifier"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceStateActionSequenceMod(Enum<Modifier>(val), devicePos,
            smPos, t, actPos, seqPos);
        break;
      }
      case "DelayValue"_hash: {
        auto val = prop.text().as_uint();
        worker->setDeviceStateActionSequenceDelayMs(val, devicePos, smPos, t,
            actPos, seqPos);
        break;
      }
      case "State"_hash:
      case "StateName"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceStateActionSequenceStateName(
            Enum<StateMachineType>(val), devicePos, smPos, t, actPos, seqPos);
        break;
      }
      case "Value"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceStateActionSequenceStateValue(val, devicePos, smPos, t,
            actPos, seqPos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import device action: unknown property (name = %1, value = %2)").arg(
                QString::fromStdString(name)).arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
        worker->setDeviceStateActionUnknownParam(unknown, devicePos, smPos, t,
            actPos, seqPos);
        break;
      }
    }
  }
  return true;
}

bool ConfigH900::readNumeric(pugi::xml_node &numeric)
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
  auto finish = numeric.child("Finish");
  if (finish) {
    ret &= worker->addDeviceNumpadDigitsCommand(devicePos,
        item::DigitSection::Finish);
    ret &= readNumericActionSequences(finish, devicePos,
        item::DigitSection::Finish, 0);
  }
  return ret;
}

bool ConfigH900::readNumericActions(pugi::xml_node &actions,
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
            QString::fromStdString(c->getDevices().back().label.get())),
        ContentType::PlainText);
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

bool ConfigH900::readNumericActionSequences(pugi::xml_node &action,
    uint32_t devicePos, item::DigitSection s, uint32_t digit)
{
  auto ret = true;

  for (pugi::xml_node prop : action.children("Action")) {
    ret &= readNumericActionSequence(prop, devicePos, s, digit);
  }
  return ret;
}

bool ConfigH900::readNumericActionSequence(pugi::xml_node &sequence,
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

bool ConfigH900::readActionSequenceData(pugi::xml_node &sequence,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
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
            Enum<StateMachineType>(val), devicePos, s, digit, seqPos);
        break;
      }
      case "Value"_hash: {
        auto val = prop.text().as_string();
        worker->setDeviceNumpadActionSequenceStateValue(val, devicePos, s,
            digit, seqPos);
        break;
      }
      default: {
        auto unknown = toUnknownElement(prop);
        emit writeLog(LogLevel::Debug,
            tr("import device action: unknown property (name = %1, value = %2)").arg(
                QString::fromStdString(name)).arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
        worker->setDeviceNumpadActionUnknownParam(unknown, devicePos, s, digit,
            seqPos);
        break;
      }
    }
  }
  return true;
}

bool ConfigH900::readIrList(pugi::xml_node &commands)
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
                QString::fromStdString(name)).arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
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

bool ConfigH900::readIr(pugi::xml_node &command)
{
  auto devicePos = c->getDevices().size() - 1;

  auto name = command.child("Name").child_value();
  auto data = command.child("Data");
  auto protocol = data.child("Protocol").text().as_int(); // -1 -> escape raw cmd
  auto code = string(data.child("Code").child_value());
  auto rawData = lib::hexStringToBytes(code);
  if (protocol < 0) {
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
    cmd.streamIndex.set(lib::parseHarmony16_file(rawData[0], rawData[1]));
    return worker->setIrCommand(devicePos, cmd);
  } else {
    //protocol command
    item::ProtoCommand cmd;
    cmd.name.set(name);
    cmd.protocolIndex.set(protocol);
    cmd.data.set(rawData);
    return worker->setIrCommand(devicePos, cmd);
  }
}

bool ConfigH900::readActivities(pugi::xml_node &root)
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

bool ConfigH900::readActivitiy(pugi::xml_node &activity, uint32_t id)
{
  addId(id);
  auto ret = worker->addActivityCommand(-1, id); //append
  if (!ret) {
    return false;
  }
  auto pos = c->getActivities().size() - 1;

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
                QString::fromStdString(name)).arg(
                QString::fromStdString(unknown.text)), ContentType::PlainText);
        worker->setDeviceUnknownProperty(unknown, pos);
        break;
      }
    }
  }

  ret = true;
  for (pugi::xml_node prop : activity.children("ControlGroup")) {
    auto name = string(prop.attribute("name").as_string());
    auto h = lib::hash_fnv1a(name.data(), name.size());
    switch (h) {
      case "Misc"_hash: {
        ret &= readActivityButtons(prop, item::ButtonType::Soft);
        break;
      }
      case "HardButtons"_hash: {
        ret &= readActivityButtons(prop, item::ButtonType::Hard);
        break;
      }
    }
  }
//  auto states = activity.child("States");
//  ret &= readStatemachines(states);
//  auto numeric = activity.child("Numeric");
//  if (numeric) {
//    ret &= readNumeric(numeric);
//  }
//  auto commands = activity.child("Commands");
//  ret &= readIrList(commands); todo
  return ret;
}

bool ConfigH900::readActivityButtons(pugi::xml_node &buttons,
    enum item::ButtonType t)
{
  auto ret = true;
  for (pugi::xml_node prop : buttons.children("Button")) {
    ret &= readActivityButton(prop, t);
  }
  return ret;
}

bool ConfigH900::readActivityButton(pugi::xml_node &button,
    enum item::ButtonType t)
{
//  int buttonPos;
//
//  auto activityPos = c->getActivities().size() - 1;
//
//  auto ret = worker->addActivityButtonCommand(t, activityPos, -1); //append
//  if (!ret) {
//    return false;
//  }
//  buttonPos = c->getActivities()[activityPos].getButtons().size() - 1;
//
//  auto actionId = string(
//      button.child("ActionId").text().as_string("1_unknown_Hold"));
//  auto action = item::Button::getAction(actionId);
//  worker->setActivityButtonAction(action, activityPos, buttonPos);
//
//  //hard/soft are different
//  auto name = button.attribute("name");
//  if (name) {
//    auto c = name.value();
//    worker->setActivityButtonName(string(c), activityPos, buttonPos);
//  } else {
//    auto c = button.child("Label").child_value();
//    worker->setActivityButtonName(string(c), activityPos, buttonPos);
//  }
//
//  //only soft buttons
//  auto pos = button.child("Position");
//  if (pos) {
//    auto p = pos.text().as_uint(0);
//    worker->setActivityButtonPosition(p, activityPos, buttonPos);
//  }
//  auto file = button.child("Icon");
//  if (file) {
//    auto c = name.value();
//    worker->setActivityButtonFile(string(c), activityPos, buttonPos);
//  }
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

bool ConfigH900::readIrProto()
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

bool ConfigH900::readIrStream()
{
  binary::ssIr::File streams;
  auto status = streams.parse(QString(wp + "/" + ssIrPath).toStdString());
  if (status != binary::ssIr::Status::OK) {
    emit writeLog(LogLevel::Error,
        tr("import ir stream parser error: %d").arg((int) status),
        ContentType::PlainText);
    return false;
  }
  auto ret = worker->setIrStreams(streams);
  emit writeLog(LogLevel::Debug,
      tr("read %1 binary ir streams").arg(c->getStreamLib().getStreamCount()),
      ContentType::PlainText);
  return ret;
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
  property.text().set(hash);
  property = properties.append_child("Property");
  property.append_attribute("name").set_value("LastUpdated");
  property.text().set(writerTime);
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
  ret &= writeDeviceButtons(softButtons, data.getId(), data.getButtons(), item::ButtonType::Soft);
  auto hardButtons = presentation.append_child("ControlGroup");
  hardButtons.append_attribute("name").set_value("HardButtons");
  ret &= writeDeviceButtons(hardButtons, data.getId(), data.getButtons(), item::ButtonType::Hard);

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

  auto states = device.append_child("States");
  ret &= writeStatemachines(states, data.getId(), data.getStateMachines());
  if (data.getNumpad().has_value()) {
    auto numeric = device.append_child("Numeric");
    ret &= writeNumeric(numeric, data.getId(), data.getNumpad().value());
  }
  auto commands = device.append_child("Commands");
  ret &= writeIrList(commands, data.getIrCommands());
// @formatter:on
  return ret;
}

bool ConfigH900::writeDeviceButtons(pugi::xml_node &buttons, uint32_t deviceId,
    const vector<item::Button> &data, enum item::ButtonType t)
{
  bool ret = true;

  for (const auto &d : data) {
    if (t != d.getButtonType()) {
      continue;
    }
    auto button = buttons.append_child("Button");
    ret &= writeDeviceButton(button, deviceId, d);
  }
  return ret;
}

bool ConfigH900::writeDeviceButton(pugi::xml_node &button, uint32_t deviceId,
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

  if (data.resetAction.has_value()) {
    auto actionType = action.append_child(
        data.resetAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.resetAction,
        item::StateTransitionType::Relative);
  }
  if (data.nextStateAction.has_value()) {
    auto actionType = action.append_child(
        data.nextStateAction->actionType.get().getString());
    ret &= writeDeviceAction(actionType, deviceId, *data.nextStateAction,
        item::StateTransitionType::Relative);
  }
  if (data.prevStateAction.has_value()) {
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

bool ConfigH900::writeNumeric(pugi::xml_node &numeric, uint32_t deviceId,
    const item::Numpad &data)
{
  bool ret = true;

  if (data.fixedDigits.isIncluded() == Include::ALWAYS) {
    numeric.append_child("FixedDigits").text().set(data.fixedDigits.get());
  }
  if (data.finish.has_value()) {
    auto actionType = numeric.append_child("Finish");
    ret &= writeDeviceAction(actionType, deviceId, *data.finish,
        item::StateTransitionType::Discrete); //dummy
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

bool ConfigH900::writeNumericActions(pugi::xml_node &action, uint32_t deviceId,
    const item::Digits &data)
{
  bool ret = true;

  for (int i = 0; i < data.size(); i++) {
    auto actionType = action.append_child("Digit");
    actionType.append_attribute("value").set_value(to_string(i));
    ret &= writeDeviceAction(actionType, deviceId, data[i],
        item::StateTransitionType::Discrete); //dummy
  }
  return ret;
}

bool ConfigH900::writeIrList(pugi::xml_node &commands,
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
    auto command = commands.append_child("Command");
    ret &= writeIr(command, data.getRawCommands()[i]);
  }
  for (int i = 0; i < data.getProtoCommands().size(); i++) {
    auto command = commands.append_child("Command");
    ret &= writeIr(command, data.getProtoCommands()[i]);
  }
  return ret;
}

bool ConfigH900::writeIr(pugi::xml_node &command, const item::RawCommand &data)
{
  vector<uint8_t> raw { 0xff, 0xff }; //start with 0xffff

  command.append_child("Name").text().set(data.name.get());
  auto cmdData = command.append_child("Data");
  cmdData.append_child("Protocol").text().set(-1);
  lib::setHarmony16_file(data.streamIndex.get(), raw);
  cmdData.append_child("Code").text().set(lib::bytesToHexString(raw));
  return true;
}

bool ConfigH900::writeIr(pugi::xml_node &command,
    const item::ProtoCommand &data)
{
  command.append_child("Name").text().set(data.name.get());
  auto cmdData = command.append_child("Data");
  cmdData.append_child("Protocol").text().set(data.protocolIndex.get());
  const auto &raw = data.data.get();
  cmdData.append_child("Code").text().set(lib::bytesToHexString(raw));
  return true;
}

bool ConfigH900::writeActivities(pugi::xml_node &root)
{
  bool ret = true;

  //todo activity -1 power off

  const auto &activities = c->getActivities();
  for (const auto &data : activities) {
    auto activity = root.append_child("Activity");
    ret &= writeActivity(activity, data);
  }
  return ret;
}

bool ConfigH900::writeActivity(pugi::xml_node &activity,
    const data::item::Activity &data)
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
  auto softButtons = presentation.append_child("ControlGroup");
  softButtons.append_attribute("name").set_value("Misc");
  ret &= writeActivityButtons(softButtons, data.getButtons(), item::ButtonType::Soft);
  auto hardButtons = presentation.append_child("ControlGroup");
  hardButtons.append_attribute("name").set_value("HardButtons");
  ret &= writeActivityButtons(hardButtons, data.getButtons(), item::ButtonType::Hard);

//  auto states = activity.append_child("States");
//  ret &= writeStatemachines(states, data.getId(), data.getStateMachines());
//  if (data.getNumpad().has_value()) {
//    auto numeric = activity.append_child("Numeric");
//    ret &= writeNumeric(numeric, data.getId(), data.getNumpad().value());
//  }
//  auto commands = activity.append_child("Commands");
//  ret &= writeIrList(commands, data.getIrCommands());
// @formatter:on
  return ret;
}

bool ConfigH900::writeActivityButtons(pugi::xml_node &buttons,
    const std::vector<data::item::Button> &data, enum data::item::ButtonType t)
{
  bool ret = true;

  for (const auto &d : data) {
    if (t != d.getButtonType()) {
      continue;
    }
    auto button = buttons.append_child("Button");
    ret &= writeActivityButton(button, d);
  }
  return ret;
}

bool ConfigH900::writeActivityButton(pugi::xml_node &button,
    const data::item::Button &data)
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
  //todo button.append_child("ActionId").text().set(data.getActionId(deviceId));
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

bool ConfigH900::dumpActionListXml()
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

  QDir().mkpath(wp + "/" + QFileInfo(actionListPath).path());
#ifdef _WIN32
  ret = xml.save_file(QString(wp + "/" + actionListPath).toStdWString().c_str(),
      PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
#else
  ret = xml.save_file(QString(wp + "/" + actionListPath).toUtf8(),
      PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
#endif
  return true;
}

bool ConfigH900::exportDevices(pugi::xml_node &root)
{
  bool ret = true;

  const auto &devices = c->getDevices();
  for (const auto &data : devices) {
    ret &= exportDevice(root, data);
  }
  return ret;
}

bool ConfigH900::exportDevice(pugi::xml_node &root, const item::Device &data)
{
  return exportButtons(root, data.getId(), data.getButtons());
}

bool ConfigH900::exportButtons(pugi::xml_node &root, uint32_t deviceId,
    const std::vector<data::item::Button> &data)
{
  bool ret = true;

  for (const auto &d : data) {
    ret &= exportButton(root, deviceId, d);
  }
  return ret;
}

bool ConfigH900::exportButton(pugi::xml_node &root, uint32_t deviceId,
    const data::item::Button &data)
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

bool ConfigH900::writeIrProto()
{
  uint32_t crc;
  vector<uint8_t> tmp;

  auto status = c->getProtocolLib().serialise(
      QString(wp + "/" + irProtoPath).toStdString(), &crc);
  if (status != binary::irProto::Status::OK) {
    emit writeLog(LogLevel::Error,
        tr("write ir protocols parser error: %d").arg((int) status),
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

bool ConfigH900::writeIrStream()
{
  auto status = c->getStreamLib().serialise(
      QString(wp + "/" + ssIrPath).toStdString());
  if (status != binary::ssIr::Status::OK) {
    emit writeLog(LogLevel::Error,
        tr("write ir stream parser error: %d").arg((int) status),
        ContentType::PlainText);
    return false;
  }
  emit writeLog(LogLevel::Debug,
      tr("wrote %1 binary ir streams").arg(c->getStreamLib().getStreamCount()),
      ContentType::PlainText);
  return true;
}

}
}
