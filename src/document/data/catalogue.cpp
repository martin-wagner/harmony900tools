// SPDX-License-Identifier: LGPL-2.1-or-later

#include "catalogue.h"

#include "document/config.h"
#include "cmd/addDevice.h"
#include "cmd/removeDevice.h"
#include "cmd/removeState.h"
#include "cmd/removeActionSequence.h"
#include "cmd/addButton.h"
#include "cmd/removeButton.h"
#include "cmd/addState.h"
#include "cmd/addActionSequence.h"
#include "cmd/addStatemachine.h"
#include "cmd/removeStatemachine.h"
#include "cmd/setId.h"
#include "cmd/setUserData.h"
#include "cmd/setControllerMetadata.h"
#include "cmd/setActionData.h"
#include "cmd/setButtonData.h"
#include "cmd/addNumpad.h"
#include "cmd/removeNumpad.h"
#include "cmd/setIr.h"
#include "cmd/removeIr.h"
#include "cmd/setIrProto.h"
#include "cmd/removeIrProto.h"
#include "cmd/setUnknownProperty.h"

using namespace std;

namespace document
{
namespace data
{

CmdCatalogue::CmdCatalogue(ConfigData &c, lib::UndoStack &undo, QObject *parent) :
    QObject(parent), c(c), undo(undo), uid(lib::UidGenerator::getInstance())
{
}

bool CmdCatalogue::setUserId(uint32_t id)
{
  auto *cmd = new SetUserIdCommand(c, id);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserFirstName(const QString &v)
{
  PropertyAccess<string> access =
      {
        tr("set user first name"),
        [this]() {return c.getUser().firstName.get();},
        [this](
            const string &v) {c.getUser().firstName.set(v).setIncluded(Include::ALWAYS);},
        v.toStdString() };
  return setProperty<string>(access);
}

bool CmdCatalogue::setUserLastName(const QString &v)
{
  PropertyAccess<string> access =
      {
        tr("set user last name"),
        [this]() {return c.getUser().lastName.get();},
        [this](
            const string &v) {c.getUser().lastName.set(v).setIncluded(Include::ALWAYS);},
        v.toStdString() };
  return setProperty<string>(access);
}

bool CmdCatalogue::setUserMetadata()
{
  auto *cmd = new SetUserMetadataCommand(c);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserMetadata(const QString &user,
    const QString &creationDate, const QString &modificationDate)
{
  auto *cmd = new SetUserMetadataCommand(c, user, creationDate,
      modificationDate);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserNewDeviceFound(bool f)
{
  PropertyAccess<bool> access =
      {
        tr("set new device found"),
        [this]() {return c.getUser().newDeviceFound.get();},
        [this](
            const bool &v) {c.getUser().newDeviceFound.set(v).setIncluded(Include::ALWAYS);},
        f };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setUserTrainingWheels(bool w)
{
  PropertyAccess<bool> access =
      {
        tr("set training wheels"),
        [this]() {return c.getUser().trainingWheels.get();},
        [this](
            const bool &v) {c.getUser().trainingWheels.set(v).setIncluded(Include::ALWAYS);},
        w };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setUserLocale(const Enum<Locale> &l)
{
  PropertyAccess<Enum<Locale>> access =
      {
        tr("set locale"),
        [this]() {return c.getUser().locale.get();},
        [this](
            const Enum<Locale> &v) {c.getUser().locale.set(v).setIncluded(Include::ALWAYS);},
        l };
  return setProperty<Enum<Locale>>(access);
}

bool CmdCatalogue::setUserTimeFormat(const Enum<TimeFormat> &f)
{
  PropertyAccess<Enum<TimeFormat>> access =
      {
        tr("set time format"),
        [this]() {return c.getUser().timeFormat.get();},
        [this](
            const Enum<TimeFormat> &v) {c.getUser().timeFormat.set(v).setIncluded(Include::ALWAYS);},
        f };
  return setProperty<Enum<TimeFormat>>(access);
}

bool CmdCatalogue::setUserUnknownProperty(
    const data::item::UnknownElement &value)
{
  auto *cmd = new SetUserUnknownPropertyCommand(c, value);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setControllerId(uint32_t id)
{
  auto *cmd = new SetControllerIdCommand(c, id);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setControllerMetadata(const QString &type,
    const QString &mnf, const QString &model, const QString &label)
{
  auto *cmd = new SetControllerMetadataCommand(c, type, mnf, model, label);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setControllerUnknownProperty(
    const data::item::UnknownElement &value)
{
  auto *cmd = new SetControllerUnknownPropertyCommand(c, value);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::addDeviceCommand(int pos, uint32_t id)
{
  auto *cmd = new AddDeviceCommand(c, id, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device pos %1 already exists, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::addDeviceCommand(int pos, uint32_t *id)
{
  auto *cmd = new AddDeviceCommand(c, pos);
  if (id != nullptr) {
    *id = cmd->getUid();
  }
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::removeDeviceCommand(int pos)
{
  auto *cmd = new RemoveDeviceCommand(c, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device pos %1 doesn't exist, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceType(const Enum<DeviceType> &v, uint32_t pos)
{
  PropertyAccess<Enum<DeviceType>> access =
      {
        tr("set device type"),
        [this, pos]() {return c.getDevices()[pos].type.get();},
        [this, pos](
            const Enum<DeviceType> &v) {c.getDevices()[pos].type.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<DeviceType>>(access);
}

bool CmdCatalogue::setDeviceMnf(const QString &v, uint32_t pos)
{
  PropertyAccess<string> access =
      {
        tr("set manufacturer"),
        [this, pos]() {return c.getDevices()[pos].mnf.get();},
        [this, pos](
            const string &v) {c.getDevices()[pos].mnf.set(v).setIncluded(Include::ALWAYS);},
        v.toStdString() };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceModel(const QString &v, uint32_t pos)
{
  PropertyAccess<string> access =
      {
        tr("set model"),
        [this, pos]() {return c.getDevices()[pos].model.get();},
        [this, pos](
            const string &v) {c.getDevices()[pos].model.set(v).setIncluded(Include::ALWAYS);},
        v.toStdString() };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceLabel(const QString &v, uint32_t pos)
{
  PropertyAccess<string> access =
      {
        tr("set label"),
        [this, pos]() {return c.getDevices()[pos].label.get();},
        [this, pos](
            const string &v) {c.getDevices()[pos].label.set(v).setIncluded(Include::ALWAYS);},
        v.toStdString() };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceManualPower(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set manual power"),
        [this, pos]() {return c.getDevices()[pos].manualPower.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].manualPower.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceAlwaysOn(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set always on"),
        [this, pos]() {return c.getDevices()[pos].alwaysOn.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].alwaysOn.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceAutoPower(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set auto power"),
        [this, pos]() {return c.getDevices()[pos].autoPower.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].autoPower.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceAudioSwitch(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set audio switch"),
        [this, pos]() {return c.getDevices()[pos].audioSwitch.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].audioSwitch.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceDimmer(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set dimmer"),
        [this, pos]() {return c.getDevices()[pos].dimmer.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].dimmer.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceHasBands(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set has bands"),
        [this, pos]() {return c.getDevices()[pos].hasBands.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].hasBands.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceHasPresets(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set has presets"),
        [this, pos]() {return c.getDevices()[pos].hasPresets.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].hasPresets.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceIsNewDevice(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set is new device"),
        [this, pos]() {return c.getDevices()[pos].isNewDevice.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].isNewDevice.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceIsDisplayDevice(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set is display device"),
        [this, pos]() {return c.getDevices()[pos].isDisplayDevice.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].isDisplayDevice.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceMenuOnDevice(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set menu on device"),
        [this, pos]() {return c.getDevices()[pos].menuOnDevice.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].menuOnDevice.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceOnScreenGuide(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set on screen guide"),
        [this, pos]() {return c.getDevices()[pos].onScreenGuide.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].onScreenGuide.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceRecordMediaFixedDisc(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set record media fixed disc"),
        [this, pos]() {return c.getDevices()[pos].recordMediaFixedDisc.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].recordMediaFixedDisc.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceRecordMediaRemovableVideotape(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set record media removable videotape"),
        [this, pos]() {return c.getDevices()[pos].recordMediaRemovableVideotape.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].recordMediaRemovableVideotape.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceRevertInput(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set revert input"),
        [this, pos]() {return c.getDevices()[pos].revertInput.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].revertInput.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceScart(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set scart"),
        [this, pos]() {return c.getDevices()[pos].scart.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].scart.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceVideoSwitch(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set video switch"),
        [this, pos]() {return c.getDevices()[pos].videoSwitch.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].videoSwitch.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceNumDiscs(int32_t v, uint32_t pos)
{
  PropertyAccess<int32_t> access =
      {
        tr("set number of discs"),
        [this, pos]() {return c.getDevices()[pos].numDiscs.get();},
        [this, pos](
            const int32_t &v) {c.getDevices()[pos].numDiscs.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<int32_t>(access);
}

bool CmdCatalogue::setDeviceNumLights(int32_t v, uint32_t pos)
{
  PropertyAccess<int32_t> access =
      {
        tr("set number of lights"),
        [this, pos]() {return c.getDevices()[pos].numLights.get();},
        [this, pos](
            const int32_t &v) {c.getDevices()[pos].numLights.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<int32_t>(access);
}

bool CmdCatalogue::setDevicePvrType(const Enum<PvrType> &v, uint32_t pos)
{
  PropertyAccess<Enum<PvrType>> access =
      {
        tr("set pvr type"),
        [this, pos]() {return c.getDevices()[pos].pvrType.get();},
        [this, pos](
            const Enum<PvrType> &v) {c.getDevices()[pos].pvrType.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<PvrType>>(access);
}

bool CmdCatalogue::setDeviceTunerInput(const Enum<TunerInput> &v, uint32_t pos)
{
  PropertyAccess<Enum<TunerInput>> access =
      {
        tr("set tuner input"),
        [this, pos]() {return c.getDevices()[pos].tunerInput.get();},
        [this, pos](
            const Enum<TunerInput> &v) {c.getDevices()[pos].tunerInput.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<TunerInput>>(access);
}

bool CmdCatalogue::setDeviceUnknownProperty(
    const data::item::UnknownElement &value, uint32_t pos)
{
  auto *cmd = new SetDeviceUnknownPropertyCommand(c, value, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::addDeviceButtonCommand(item::ButtonType t,
    uint32_t devicePos, int buttonPos)
{
  auto *cmd = new AddButtonCommand(c, t, devicePos, buttonPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: button pos %1/%2 already exists, dropped").arg(devicePos).arg(
            buttonPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeDeviceButtonCommand(item::ButtonType t,
    uint32_t devicePos, int buttonPos)
{
  auto *cmd = new RemoveButtonCommand(c, t, devicePos, buttonPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: button pos %1/%2 doesn't exist, dropped").arg(devicePos).arg(
            buttonPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceButtonAction(const string &v, item::ButtonType t,
    uint32_t devicePos, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button action"),
        [this, t, devicePos, buttonPos]() {return getButtonFromDeviceRef(c, t, devicePos, buttonPos).action.get();},
        [this, t, devicePos, buttonPos](
            const string &v) {getButtonFromDeviceRef(c, t, devicePos, buttonPos).action.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceButtonName(const string &v, item::ButtonType t,
    uint32_t devicePos, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button name"),
        [this, t, devicePos, buttonPos]() {return getButtonFromDeviceRef(c, t, devicePos, buttonPos).name.get();},
        [this, t, devicePos, buttonPos](
            const string &v) {getButtonFromDeviceRef(c, t, devicePos, buttonPos).name.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceButtonFile(const string &v, item::ButtonType t,
    uint32_t devicePos, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button file"),
        [this, t, devicePos, buttonPos]() {return getButtonFromDeviceRef(c, t, devicePos, buttonPos).file.get();},
        [this, t, devicePos, buttonPos](
            const string &v) {getButtonFromDeviceRef(c, t, devicePos, buttonPos).file.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceButtonPosition(const int32_t &v, item::ButtonType t,
    uint32_t devicePos, int buttonPos)
{
  PropertyAccess<int32_t> access =
      {
        tr("set button position"),
        [this, t, devicePos, buttonPos]() {return getButtonFromDeviceRef(c, t, devicePos, buttonPos).position.get();},
        [this, t, devicePos, buttonPos](
            const int32_t &v) {getButtonFromDeviceRef(c, t, devicePos, buttonPos).position.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<int32_t>(access);
}

bool CmdCatalogue::addDeviceStatemachineCommand(uint32_t devicePos, int smPos)
{
  auto *cmd = new AddStatemachineCommand(c, devicePos, smPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: statemachine pos %1/%2 failed, dropped").arg(devicePos).arg(
            smPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeDeviceStatemachineCommand(uint32_t devicePos,
    int smPos)
{
  auto *cmd = new RemoveIrCommand(c, devicePos, smPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: statemachine pos %1/%2 doesn't exist, dropped").arg(
            devicePos).arg(smPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceStatemachineType(const Enum<StateMachineType> &type,
    uint32_t devicePos, int smPos)
{
  PropertyAccess<Enum<StateMachineType>> access =
      {
        tr("set state machine type"),
        [this, devicePos, smPos]() {return c.getDevices()[devicePos].getStateMachines()[smPos].smType.get();},
        [this, devicePos, smPos](
            const Enum<StateMachineType> &v) {c.getDevices()[devicePos].getStateMachines()[smPos].smType.set(v).setIncluded(Include::ALWAYS);},
        type };
  return setProperty<Enum<StateMachineType>>(access);
}

bool CmdCatalogue::setDeviceStatemachineDelay(uint32_t delayMs,
    uint32_t devicePos, int smPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set state machine delay"),
        [this, devicePos, smPos]() {return c.getDevices()[devicePos].getStateMachines()[smPos].delayMs.get();},
        [this, devicePos, smPos](
            const uint32_t &v) {c.getDevices()[devicePos].getStateMachines()[smPos].delayMs.set(v).setIncluded(Include::ALWAYS);},
        delayMs };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::addDeviceSmStateCommand(uint32_t devicePos, uint32_t smPos,
    item::StateTransitionType t, const QString &name, int actPos)
{
  auto *cmd = new AddStateCommand(c, devicePos, smPos, name, t, actPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device state pos %1/%2/%3 failed, dropped").arg(devicePos).arg(
            smPos).arg(actPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::addDeviceSmActionCommand(uint32_t devicePos, uint32_t smPos,
    item::StateTransitionAction t)
{
  auto *cmd = new AddActionCommand(c, devicePos, smPos, t);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device action pos %1/%2/%3 failed, dropped").arg(devicePos).arg(
            smPos).arg((int) t), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeDeviceSmStateCommand(uint32_t devicePos,
    uint32_t smPos, item::StateTransitionType t, int actPos)
{
  auto *cmd = new RemoveStateCommand(c, devicePos, smPos, t, actPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device action pos %1/%2/%3 doesn't exist, dropped").arg(
            devicePos).arg(smPos).arg(actPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeDeviceSmActionCommand(uint32_t devicePos,
    uint32_t smPos, item::StateTransitionAction t)
{
  auto *cmd = new RemoveActionCommand(c, devicePos, smPos, t);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device action pos %1/%2/%3 doesn't exist, dropped").arg(
            devicePos).arg(smPos).arg((int) t), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceSmActionType(const Enum<ActionType> &v,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos)
{
  PropertyAccess<Enum<ActionType>> access =
      {
        tr("set device action type"),
        [this, devicePos, smPos, t, actPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->actionType.get();},
        [this, devicePos, smPos, t, actPos](
            const Enum<ActionType> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->actionType.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<ActionType>>(access);
}

bool CmdCatalogue::setDeviceSmActionRepeatWillNotHarm(bool v,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos)
{
  PropertyAccess<bool> access =
      {
        tr("set device action repeat will not harm"),
        [this, devicePos, smPos, t, actPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->repeatWillNotHarm.get();},
        [this, devicePos, smPos, t, actPos](
            const bool &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->repeatWillNotHarm.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::addDeviceStateActionSequenceCommand(uint32_t devicePos,
    uint32_t smPos, uint32_t actPos, item::StateTransitionAction t, int seqPos)
{
  auto *cmd = new AddDeviceActionSequenceCommand(c, devicePos, smPos, actPos, t,
      seqPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device act-seq pos %1/%2a/%3/%4 failed, dropped").arg(
            devicePos).arg(smPos).arg(actPos).arg(seqPos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeDeviceStateActionSequenceCommand(uint32_t devicePos,
    uint32_t smPos, uint32_t actPos, item::StateTransitionAction t,
    uint32_t seqPos)
{
  auto *cmd = new RemoveDeviceActionSequenceCommand(c, devicePos, smPos, actPos,
      t, seqPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device act-seq pos %1/%2a/%3/%4 doesn't exist, dropped").arg(
            devicePos).arg(smPos).arg(actPos).arg(seqPos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceStateActionSequenceOp(const Enum<Operation> &v,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<Enum<Operation>> access =
      {
        tr("set device action sequence opcode"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].opcode.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const Enum<Operation> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].opcode.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<Operation>>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceCmd(const string &v,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set device action sequence cmd"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].cmd.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const string &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].cmd.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceDelayMs(uint32_t v,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set device action sequence delay ms"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].delayMs.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const uint32_t &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].delayMs.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceStateName(
    const Enum<StateMachineType> &v, uint32_t devicePos, uint32_t smPos,
    item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<Enum<StateMachineType>> access =
      {
        tr("set device action sequence state name"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateName.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const Enum<StateMachineType> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateName.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<StateMachineType>>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceStateValue(const string &v,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set device action sequence state v"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateValue.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const string &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateValue.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceMod(const Enum<Modifier> &v,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<Enum<Modifier>> access =
      {
        tr("set device action sequence modifier"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].mod.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const Enum<Modifier> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].mod.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<Modifier>>(access);
}

bool CmdCatalogue::setDeviceStateActionUnknownParam(
    const data::item::UnknownElement &v, uint32_t devicePos, uint32_t smPos,
    item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos)
{
  auto *cmd = new SetDeviceStateActionUnknownParamCommand(c, v, devicePos,
      smPos, actPos, t, seqPos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::addDeviceNumpadCommand(uint32_t devicePos)
{
  auto *cmd = new AddNumpadCommand(c, devicePos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device add numpad pos %1 failed").arg(devicePos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeDeviceNumpadCommand(uint32_t devicePos)
{
  auto *cmd = new RemoveNumpadCommand(c, devicePos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: removing numpad pos %1 failed, dropped").arg(devicePos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::addDeviceNumpadDigitsCommand(uint32_t devicePos,
    item::DigitSection s)
{
  auto *cmd = new AddDigitsCommand(c, devicePos, s);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device add numpad digits pos %1 failed").arg(devicePos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeDeviceNumpadDigitsCommand(uint32_t devicePos,
    item::DigitSection s)
{
  auto *cmd = new RemoveDigitsCommand(c, devicePos, s);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: removing numpad digits pos %1 failed, dropped").arg(
            devicePos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceNumpadFixedDigits(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set numpad fixed digits"),
        [this, devicePos]() {return c.getDevices()[devicePos].getNumpad()->fixedDigits.get();},
        [this, devicePos](
            const uint32_t &v) {c.getDevices()[devicePos].getNumpad()->fixedDigits.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setDeviceNumpadActionRepeatWillNotHarm(bool v,
    uint32_t devicePos, item::DigitSection s, uint32_t digit)
{
  PropertyAccess<bool> access =
      {
        tr("set device action repeat will not harm"),
        [this, devicePos, s, digit]() {return getActionFromNumpadRef(c, devicePos, s, digit)->repeatWillNotHarm.get();},
        [this, devicePos, s, digit](
            const bool &v) {getActionFromNumpadRef(c, devicePos, s, digit)->repeatWillNotHarm.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<bool>(access);
}

bool CmdCatalogue::addDeviceNumpadActionSequenceCommand(uint32_t devicePos,
    item::DigitSection s, uint32_t digit, int seqPos)
{
  auto *cmd = new AddDeviceActionSequenceCommand(c, devicePos, s, digit,
      seqPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device act-seq pos %1/%2b/%3/%4 failed, dropped").arg(
            devicePos).arg((int) s).arg(digit).arg(seqPos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeDeviceNumpadActionSequenceCommand(uint32_t devicePos,
    item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  auto *cmd = new RemoveDeviceActionSequenceCommand(c, devicePos, s, digit,
      seqPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device act-seq pos %1/%2b/%3/%4 doesn't exist, dropped").arg(
            devicePos).arg((int) s).arg(digit).arg(seqPos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceNumpadActionSequenceOp(const Enum<Operation> &v,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<Enum<Operation>> access =
      {
        tr("set device action sequence opcode"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].opcode.get();},
        [this, devicePos, s, digit, seqPos](
            const Enum<Operation> &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].opcode.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<Operation>>(access);
}

bool CmdCatalogue::setDeviceNumpadActionSequenceCmd(const std::string &v,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set device action sequence cmd"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].cmd.get();},
        [this, devicePos, s, digit, seqPos](
            const string &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].cmd.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceNumpadActionSequenceDelayMs(uint32_t v,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set device action sequence delay ms"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].delayMs.get();},
        [this, devicePos, s, digit, seqPos](
            const uint32_t &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].delayMs.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setDeviceNumpadActionSequenceStateName(
    const Enum<StateMachineType> &v, uint32_t devicePos, item::DigitSection s,
    uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<Enum<StateMachineType>> access =
      {
        tr("set device action sequence state name"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateName.get();},
        [this, devicePos, s, digit, seqPos](
            const Enum<StateMachineType> &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateName.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<StateMachineType>>(access);
}

bool CmdCatalogue::setDeviceNumpadActionSequenceStateValue(const std::string &v,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set device action sequence state value"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateValue.get();},
        [this, devicePos, s, digit, seqPos](
            const string &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateValue.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceNumpadActionSequenceMod(const Enum<Modifier> &v,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<Enum<Modifier>> access =
      {
        tr("set device action sequence modifier"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].mod.get();},
        [this, devicePos, s, digit, seqPos](
            const Enum<Modifier> &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].mod.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<Modifier>>(access);
}

bool CmdCatalogue::setDeviceNumpadActionUnknownParam(
    const data::item::UnknownElement &v, uint32_t devicePos,
    item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  auto *cmd = new SetDeviceNumpadActionUnknownParamCommand(c, v, devicePos, s,
      digit, seqPos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setIrPressPreSilenceMs(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir press pre silence ms"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().pressPreSilenceMs.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().pressPreSilenceMs.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrPressInterKeyMs(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir press inter key ms"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().pressInterKeyMs.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().pressInterKeyMs.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrHoldPreSilenceMs(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir hold pre silence key ms"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().holdPreSilenceMs.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().holdPreSilenceMs.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrHoldInterKeyMs(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir hold inter key ms"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().holdInterKeyMs.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().holdInterKeyMs.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrCodeType(const Enum<CodeType> &v, uint32_t devicePos)
{
  PropertyAccess<Enum<CodeType>> access =
      {
        tr("set ir command code type"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().codeType.get();},
        [this, devicePos](
            const Enum<CodeType> &v) {c.getDevices()[devicePos].getIrCommands().codeType.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<Enum<CodeType>>(access);
}

bool CmdCatalogue::setIrCodeField0(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir f0"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().field0.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().field0.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrCodeField1(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir f1"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().field1.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().field1.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrCodeField2(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir f2"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().field2.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().field2.set(v).setIncluded(Include::ALWAYS);},
        v };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrUnknownProperty(const data::item::UnknownElement &v,
    uint32_t devicePos)
{
  auto *cmd = new SetIrUnknownPropertyCommand(c, v, devicePos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setIrCommand(uint32_t devicePos, item::ProtoCommand &cmd,
    int cmdPos, bool overwrite)
{
  auto *undocmd = new SetIrCommand(c, devicePos, cmd, cmdPos, overwrite);
  auto ret = undocmd->valid();
  if (ret == true) {
    connectCommand(undocmd);
    undo.push(undocmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: set ir proto in device %1 failed, dropped").arg(devicePos),
        ContentType::PlainText);
    delete undocmd;
  }
  return ret;
}

bool CmdCatalogue::setIrCommand(uint32_t devicePos, item::RawCommand &cmd,
    int cmdPos, bool overwrite)
{
  auto *undocmd = new SetIrCommand(c, devicePos, cmd, cmdPos, overwrite);
  auto ret = undocmd->valid();
  if (ret == true) {
    connectCommand(undocmd);
    undo.push(undocmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: set ir raw in device %1 failed, dropped").arg(devicePos),
        ContentType::PlainText);
    delete undocmd;
  }
  return ret;
}

bool CmdCatalogue::removeIrProtoCommand(uint32_t devicePos, uint32_t cmdPos)
{
  auto *cmd = new RemoveIrProtoCommand(c, devicePos, cmdPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: remove ir proto in device %1 failed, dropped").arg(
            devicePos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeIrRawCommand(uint32_t devicePos, uint32_t cmdPos)
{
  auto *cmd = new RemoveIrRawCommand(c, devicePos, cmdPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: remove ir raw in device %1 failed, dropped").arg(devicePos),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::setIrProtoLib(const binary::irProto::File &file)
{
  auto *cmd = new SetIrProtoLibCommand(c, file);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::addIrProtoLibItem(const binary::irProto::IrProto &prot,
    int pos)
{
  auto *cmd = new AddIrProtoLibItemCommand(c, prot, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: add ir proto to lib failed, dropped"),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeIrProtoLibItem(int pos)
{
  auto *cmd = new RemoveIrProtoLibItemCommand(c, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: remove ir proto from lib failed, dropped"),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

void CmdCatalogue::connectCommand(BaseCommand *cmd)
{
  //connect signals. not all commands actually use all signals!

  // @formatter:off
  connect(cmd, &BaseCommand::writeLog, this, &CmdCatalogue::writeLog);
  connect(cmd, &BaseCommand::writeMsg, this, &CmdCatalogue::writeMsg);
  connect(cmd, &BaseCommand::deviceChanged, this, &CmdCatalogue::deviceChanged);
  connect(cmd, &BaseCommand::deviceAboutToBeAdded, this, &CmdCatalogue::deviceAboutToBeAdded);
  connect(cmd, &BaseCommand::deviceAdded, this, &CmdCatalogue::deviceAdded);
  connect(cmd, &BaseCommand::deviceAboutToBeRemoved, this, &CmdCatalogue::deviceAboutToBeRemoved);
  connect(cmd, &BaseCommand::deviceRemoved, this, &CmdCatalogue::deviceRemoved);
  connect(cmd, &BaseCommand::activityChanged, this, &CmdCatalogue::activityChanged);
  connect(cmd, &BaseCommand::activityAboutToBeAdded, this, &CmdCatalogue::activityAboutToBeAdded);
  connect(cmd, &BaseCommand::activityAdded, this, &CmdCatalogue::activityAdded);
  connect(cmd, &BaseCommand::activityAboutToBeRemoved, this, &CmdCatalogue::activityAboutToBeRemoved);
  connect(cmd, &BaseCommand::activityRemoved, this, &CmdCatalogue::activityRemoved);
  connect(cmd, &BaseCommand::dirtyChanged, this, &CmdCatalogue::dirtyChanged);
// @formatter:on
}

}
}
