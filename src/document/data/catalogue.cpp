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
#include "cmd/addActivity.h"
#include "cmd/moveActivity.h"
#include "cmd/removeActivity.h"
#include "cmd/setIrProto.h"
#include "cmd/addChannel.h"
#include "cmd/removeChannel.h"
#include "cmd/addActivityAction.h"
#include "cmd/removeActivityAction.h"
#include "cmd/setRole.h"
#include "cmd/removeRole.h"
#include "cmd/addPowerOnDevice.h"
#include "cmd/removePowerOnDevice.h"
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
  PropertyAccess<string> access = {
    tr("set user first name"),
    [this]() {return c.getUser().firstName.get();},
    [this](
        const string &v) {c.getUser().firstName.set(v).setIncluded(Used::YES);},
    v.toStdString(),
    Item::USER };
  return setProperty<string>(access);
}

bool CmdCatalogue::setUserLastName(const QString &v)
{
  PropertyAccess<string> access = {
    tr("set user last name"),
    [this]() {return c.getUser().lastName.get();},
    [this](
        const string &v) {c.getUser().lastName.set(v).setIncluded(Used::YES);},
    v.toStdString(),
    Item::USER };
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
            const bool &v) {c.getUser().newDeviceFound.set(v).setIncluded(Used::YES);},
        f,
        Item::USER };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setUserTrainingWheels(bool w)
{
  PropertyAccess<bool> access =
      {
        tr("set training wheels"),
        [this]() {return c.getUser().trainingWheels.get();},
        [this](
            const bool &v) {c.getUser().trainingWheels.set(v).setIncluded(Used::YES);},
        w,
        Item::USER };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setUserLocale(const Enum<Locale> &l)
{
  PropertyAccess<Enum<Locale>> access =
      {
        tr("set locale"),
        [this]() {return c.getUser().locale.get();},
        [this](
            const Enum<Locale> &v) {c.getUser().locale.set(v).setIncluded(Used::YES);},
        l,
        Item::USER };
  return setProperty<Enum<Locale>>(access);
}

bool CmdCatalogue::setUserTimeFormat(const Enum<TimeFormat> &f)
{
  PropertyAccess<Enum<TimeFormat>> access =
      {
        tr("set time format"),
        [this]() {return c.getUser().timeFormat.get();},
        [this](
            const Enum<TimeFormat> &v) {c.getUser().timeFormat.set(v).setIncluded(Used::YES);},
        f,
        Item::USER };
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
            const Enum<DeviceType> &v) {c.getDevices()[pos].type.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<Enum<DeviceType>>(access);
}

bool CmdCatalogue::setDeviceMnf(const QString &v, uint32_t pos)
{
  PropertyAccess<string> access =
      {
        tr("set manufacturer"),
        [this, pos]() {return c.getDevices()[pos].mnf.get();},
        [this, pos](
            const string &v) {c.getDevices()[pos].mnf.set(v).setIncluded(Used::YES);},
        v.toStdString(),
        Item::DEVICE,
        pos };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceModel(const QString &v, uint32_t pos)
{
  PropertyAccess<string> access =
      {
        tr("set model"),
        [this, pos]() {return c.getDevices()[pos].model.get();},
        [this, pos](
            const string &v) {c.getDevices()[pos].model.set(v).setIncluded(Used::YES);},
        v.toStdString(),
        Item::DEVICE,
        pos };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceLabel(const QString &v, uint32_t pos)
{
  PropertyAccess<string> access =
      {
        tr("set label"),
        [this, pos]() {return c.getDevices()[pos].label.get();},
        [this, pos](
            const string &v) {c.getDevices()[pos].label.set(v).setIncluded(Used::YES);},
        v.toStdString(),
        Item::DEVICE,
        pos };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceManualPower(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set manual power"),
        [this, pos]() {return c.getDevices()[pos].manualPower.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].manualPower.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceAlwaysOn(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set always on"),
        [this, pos]() {return c.getDevices()[pos].alwaysOn.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].alwaysOn.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceAutoPower(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set auto power"),
        [this, pos]() {return c.getDevices()[pos].autoPower.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].autoPower.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceAudioSwitch(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set audio switch"),
        [this, pos]() {return c.getDevices()[pos].audioSwitch.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].audioSwitch.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceDimmer(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set dimmer"),
        [this, pos]() {return c.getDevices()[pos].dimmer.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].dimmer.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceHasBands(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set has bands"),
        [this, pos]() {return c.getDevices()[pos].hasBands.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].hasBands.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceHasPresets(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set has presets"),
        [this, pos]() {return c.getDevices()[pos].hasPresets.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].hasPresets.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceIsNewDevice(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set is new device"),
        [this, pos]() {return c.getDevices()[pos].isNewDevice.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].isNewDevice.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceIsDisplayDevice(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set is display device"),
        [this, pos]() {return c.getDevices()[pos].isDisplayDevice.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].isDisplayDevice.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceMenuOnDevice(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set menu on device"),
        [this, pos]() {return c.getDevices()[pos].menuOnDevice.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].menuOnDevice.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceOnScreenGuide(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set on screen guide"),
        [this, pos]() {return c.getDevices()[pos].onScreenGuide.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].onScreenGuide.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceRecordMediaFixedDisc(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set record media fixed disc"),
        [this, pos]() {return c.getDevices()[pos].recordMediaFixedDisc.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].recordMediaFixedDisc.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceRecordMediaRemovableVideotape(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set record media removable videotape"),
        [this, pos]() {return c.getDevices()[pos].recordMediaRemovableVideotape.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].recordMediaRemovableVideotape.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceRevertInput(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set revert input"),
        [this, pos]() {return c.getDevices()[pos].revertInput.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].revertInput.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceScart(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set scart"),
        [this, pos]() {return c.getDevices()[pos].scart.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].scart.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceVideoSwitch(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set video switch"),
        [this, pos]() {return c.getDevices()[pos].videoSwitch.get();},
        [this, pos](
            const bool &v) {c.getDevices()[pos].videoSwitch.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<bool>(access);
}

bool CmdCatalogue::setDeviceNumDiscs(int32_t v, uint32_t pos)
{
  PropertyAccess<int32_t> access =
      {
        tr("set number of discs"),
        [this, pos]() {return c.getDevices()[pos].numDiscs.get();},
        [this, pos](
            const int32_t &v) {c.getDevices()[pos].numDiscs.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<int32_t>(access);
}

bool CmdCatalogue::setDeviceNumLights(int32_t v, uint32_t pos)
{
  PropertyAccess<int32_t> access =
      {
        tr("set number of lights"),
        [this, pos]() {return c.getDevices()[pos].numLights.get();},
        [this, pos](
            const int32_t &v) {c.getDevices()[pos].numLights.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<int32_t>(access);
}

bool CmdCatalogue::setDevicePvrType(const Enum<PvrType> &v, uint32_t pos)
{
  PropertyAccess<Enum<PvrType>> access =
      {
        tr("set pvr type"),
        [this, pos]() {return c.getDevices()[pos].pvrType.get();},
        [this, pos](
            const Enum<PvrType> &v) {c.getDevices()[pos].pvrType.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
  return setProperty<Enum<PvrType>>(access);
}

bool CmdCatalogue::setDeviceTunerInput(const Enum<TunerInput> &v, uint32_t pos)
{
  PropertyAccess<Enum<TunerInput>> access =
      {
        tr("set tuner input"),
        [this, pos]() {return c.getDevices()[pos].tunerInput.get();},
        [this, pos](
            const Enum<TunerInput> &v) {c.getDevices()[pos].tunerInput.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE,
        pos };
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

bool CmdCatalogue::addDeviceButtonCommand(uint32_t devicePos,
    item::ButtonType t, int buttonPos)
{
  auto *cmd = new AddDeviceButtonCommand(c, devicePos, t, buttonPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: button in device %1 already exists, dropped").arg(
            devicePos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeDeviceButtonCommand(uint32_t devicePos,
    item::ButtonType t, int buttonPos)
{
  auto *cmd = new RemoveDeviceButtonCommand(c, devicePos, t, buttonPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: button in device %1 doesn't exist, dropped").arg(devicePos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceButtonAction(const std::string &v,
    uint32_t devicePos, item::ButtonType t, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button device action"),
        [this, devicePos, t, buttonPos]() {return getDeviceButton(c, devicePos, t, buttonPos)->action.get();},
        [this, devicePos, t, buttonPos](
            const string &v) {getDeviceButton(c, devicePos, t, buttonPos)->action.set(v).setIncluded(Used::YES);},
        v,
        toDeviceItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceButtonName(const std::string &v, uint32_t devicePos,
    item::ButtonType t, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button device name"),
        [this, devicePos, t, buttonPos]() {return getDeviceButton(c, devicePos, t, buttonPos)->name.get();},
        [this, devicePos, t, buttonPos](
            const string &v) {getDeviceButton(c, devicePos, t, buttonPos)->name.set(v).setIncluded(Used::YES);},
        v,
        toDeviceItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceButtonFile(const std::string &v, uint32_t devicePos,
    item::ButtonType t, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button device file"),
        [this, devicePos, t, buttonPos]() {return getDeviceButton(c, devicePos, t, buttonPos)->file.get();},
        [this, devicePos, t, buttonPos](
            const string &v) {getDeviceButton(c, devicePos, t, buttonPos)->file.set(v).setIncluded(Used::YES);},
        v,
        toDeviceItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceButtonPosition(const int32_t &v, uint32_t devicePos,
    item::ButtonType t, int buttonPos)
{
  PropertyAccess<int32_t> access =
      {
        tr("set button device position"),
        [this, devicePos, t, buttonPos]() {return getDeviceButton(c, devicePos, t, buttonPos)->position.get();},
        [this, devicePos, t, buttonPos](
            const int32_t &v) {getDeviceButton(c, devicePos, t, buttonPos)->position.set(v).setIncluded(Used::YES);},
        v,
        toDeviceItem(t),
        static_cast<uint32_t>(buttonPos) };
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
  auto *cmd = new RemoveStatemachineCommand(c, devicePos, smPos);
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

bool CmdCatalogue::setDeviceStatemachineType(
    const Enum<StateMachineDeviceType> &type, uint32_t devicePos, int smPos)
{
  PropertyAccess<Enum<StateMachineDeviceType>> access =
      {
        tr("set state machine type"),
        [this, devicePos, smPos]() {return c.getDevices()[devicePos].getStateMachines()[smPos].smType.get();},
        [this, devicePos, smPos](
            const Enum<StateMachineDeviceType> &v) {c.getDevices()[devicePos].getStateMachines()[smPos].smType.set(v).setIncluded(Used::YES);},
        type,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<Enum<StateMachineDeviceType>>(access);
}

bool CmdCatalogue::setDeviceStatemachineDelay(uint32_t delayMs,
    uint32_t devicePos, int smPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set state machine delay"),
        [this, devicePos, smPos]() {return c.getDevices()[devicePos].getStateMachines()[smPos].delayMs.get();},
        [this, devicePos, smPos](
            const uint32_t &v) {c.getDevices()[devicePos].getStateMachines()[smPos].delayMs.set(v).setIncluded(Used::YES);},
        delayMs,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::addDeviceSmStateCommand(uint32_t devicePos, uint32_t smPos,
    item::StateMachineType t, const QString &name, int actPos)
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
    item::StateMachineAction t)
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
    uint32_t smPos, item::StateMachineType t, int actPos)
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
    uint32_t smPos, item::StateMachineAction t)
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
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos)
{
  PropertyAccess<Enum<ActionType>> access =
      {
        tr("set device action type"),
        [this, devicePos, smPos, t, actPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->actionType.get();},
        [this, devicePos, smPos, t, actPos](
            const Enum<ActionType> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->actionType.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<Enum<ActionType>>(access);
}

bool CmdCatalogue::setDeviceSmActionRepeatWillNotHarm(bool v,
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos)
{
  PropertyAccess<bool> access =
      {
        tr("set device action repeat will not harm"),
        [this, devicePos, smPos, t, actPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->repeatWillNotHarm.get();},
        [this, devicePos, smPos, t, actPos](
            const bool &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->repeatWillNotHarm.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<bool>(access);
}

bool CmdCatalogue::addDeviceStateActionSequenceCommand(uint32_t devicePos,
    uint32_t smPos, uint32_t actPos, item::StateMachineAction t, int seqPos)
{
  auto *cmd = new AddActionSequenceCommand(c, devicePos, smPos, actPos, t,
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
    uint32_t smPos, uint32_t actPos, item::StateMachineAction t,
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
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<Enum<Operation>> access =
      {
        tr("set device action sequence opcode"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].opcode.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const Enum<Operation> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].opcode.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<Enum<Operation>>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceCmd(const string &v,
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set device action sequence cmd"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].cmd.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const string &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].cmd.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceDelayMs(uint32_t v,
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set device action sequence delay ms"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].delayMs.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const uint32_t &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].delayMs.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceStateName(
    const Enum<StateMachineDeviceType> &v, uint32_t devicePos, uint32_t smPos,
    item::StateMachineAction t, uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<Enum<StateMachineDeviceType>> access =
      {
        tr("set device action sequence state name"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateName.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const Enum<StateMachineDeviceType> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateName.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<Enum<StateMachineDeviceType>>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceStateValue(const string &v,
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set device action sequence state v"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateValue.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const string &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateValue.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setDeviceStateActionSequenceMod(const Enum<Modifier> &v,
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos, uint32_t seqPos)
{
  PropertyAccess<Enum<Modifier>> access =
      {
        tr("set device action sequence modifier"),
        [this, devicePos, smPos, t, actPos, seqPos]() {return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].mod.get();},
        [this, devicePos, smPos, t, actPos, seqPos](
            const Enum<Modifier> &v) {getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].mod.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_STATEMACHINE,
        static_cast<uint32_t>(smPos) };
  return setProperty<Enum<Modifier>>(access);
}

bool CmdCatalogue::setDeviceStateActionUnknownParam(
    const data::item::UnknownElement &v, uint32_t devicePos, uint32_t smPos,
    item::StateMachineAction t, uint32_t actPos, uint32_t seqPos)
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
            const uint32_t &v) {c.getDevices()[devicePos].getNumpad()->fixedDigits.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
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
            const bool &v) {getActionFromNumpadRef(c, devicePos, s, digit)->repeatWillNotHarm.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
  return setProperty<bool>(access);
}

bool CmdCatalogue::addDeviceNumpadActionSequenceCommand(uint32_t devicePos,
    item::DigitSection s, uint32_t digit, int seqPos)
{
  auto *cmd = new AddActionSequenceCommand(c, devicePos, s, digit, seqPos);
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
            const Enum<Operation> &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].opcode.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
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
            const string &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].cmd.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
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
            const uint32_t &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].delayMs.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setDeviceNumpadActionSequenceStateName(
    const Enum<StateMachineDeviceType> &v, uint32_t devicePos,
    item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<Enum<StateMachineDeviceType>> access =
      {
        tr("set device action sequence state name"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateName.get();},
        [this, devicePos, s, digit, seqPos](
            const Enum<StateMachineDeviceType> &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateName.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
  return setProperty<Enum<StateMachineDeviceType>>(access);
}

bool CmdCatalogue::setDeviceNumpadActionSequenceStateValue(const std::string &v,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set device action sequence state value"),
        [this, devicePos, s, digit, seqPos]() {return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateValue.get();},
        [this, devicePos, s, digit, seqPos](
            const string &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].stateValue.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
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
            const Enum<Modifier> &v) {getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].mod.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_NUMPAD };
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
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().pressPreSilenceMs.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrPressInterKeyMs(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir press inter key ms"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().pressInterKeyMs.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().pressInterKeyMs.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrHoldPreSilenceMs(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir hold pre silence key ms"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().holdPreSilenceMs.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().holdPreSilenceMs.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrHoldInterKeyMs(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir hold inter key ms"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().holdInterKeyMs.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().holdInterKeyMs.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrDefaultCodeType(const Enum<CodeType> &v,
    uint32_t devicePos)
{
  PropertyAccess<Enum<CodeType>> access =
      {
        tr("set ir command code type"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().defaultCodeType.get();},
        [this, devicePos](
            const Enum<CodeType> &v) {c.getDevices()[devicePos].getIrCommands().defaultCodeType.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
  return setProperty<Enum<CodeType>>(access);
}

bool CmdCatalogue::setIrCodeField0(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir f0"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().field0.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().field0.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrCodeField1(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir f1"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().field1.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().field1.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setIrCodeField2(uint32_t v, uint32_t devicePos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set ir f2"),
        [this, devicePos]() {return c.getDevices()[devicePos].getIrCommands().field2.get();},
        [this, devicePos](
            uint32_t v) {c.getDevices()[devicePos].getIrCommands().field2.set(v).setIncluded(Used::YES);},
        v,
        Item::DEVICE_IR };
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

bool CmdCatalogue::addActivityCommand(int pos, uint32_t id)
{
  auto *cmd = new AddActivityCommand(c, id, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: activity pos %1 already exists, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::addActivityCommand(int pos, uint32_t *id)
{
  auto *cmd = new AddActivityCommand(c, pos);
  if (id != nullptr) {
    *id = cmd->getUid();
  }
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::moveActivityCommand(uint32_t currentPos, uint32_t newPos)
{
  auto *cmd = new MoveActivityCommand(c, currentPos, newPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: activity move from/beyond end, dropped"),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeActivityCommand(int pos)
{
  auto *cmd = new RemoveActivityCommand(c, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: activity pos %1 doesn't exist, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setActivityType(const Enum<ActivityType> &v, uint32_t pos)
{
  PropertyAccess<Enum<ActivityType>> access =
      {
        tr("set device type"),
        [this, pos]() {return c.getActivities()[pos].type.get();},
        [this, pos](
            const Enum<ActivityType> &v) {c.getActivities()[pos].type.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY,
        pos };
  return setProperty<Enum<ActivityType>>(access);
}

bool CmdCatalogue::setActivityLabel(const QString &v, uint32_t pos)
{
  PropertyAccess<string> access =
      {
        tr("set label"),
        [this, pos]() {return c.getActivities()[pos].label.get();},
        [this, pos](
            const string &v) {c.getActivities()[pos].label.set(v).setIncluded(Used::YES);},
        v.toStdString(),
        Item::ACTIVITY,
        pos };
  return setProperty<string>(access);
}

bool CmdCatalogue::setActivityPvrType(const Enum<ActivityStartPage> &v,
    uint32_t pos)
{
  PropertyAccess<Enum<ActivityStartPage>> access = { tr("set pvr type"), [this,
      pos]() {return c.getActivities()[pos].pvrType.get();}, [this, pos](
      const Enum<ActivityStartPage> &v) {
        c.getActivities()[pos].pvrType.set(v).setIncluded(Used::YES);
      }, v, Item::ACTIVITY, pos };

  return setProperty<Enum<ActivityStartPage>>(access);
}

bool CmdCatalogue::setActivityControlGroupHardButtons(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set control group hard buttons"),
        [this, pos]() {return c.getActivities()[pos].controlGroup_HardButtons.get();},
        [this, pos](
            const bool &v) {
              c.getActivities()[pos].controlGroup_HardButtons.set(v).setIncluded(Used::YES);
            },
        v,
        Item::ACTIVITY,
        pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityPowerOffUnusedDevices(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = { tr("set power off unused Activitys"), [this,
      pos]() {return c.getActivities()[pos].powerOffUnusedDevices.get();}, [
      this, pos](const bool &v) {
    c.getActivities()[pos].powerOffUnusedDevices.set(v).setIncluded(Used::YES);
  }, v, Item::ACTIVITY, pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityTrainingWheels(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set training wheels"),
    [this, pos]() {return c.getActivities()[pos].trainingWheels.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].trainingWheels.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityUnusedDevicesHelp(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set unused Activitys help"),
    [this, pos]() {return c.getActivities()[pos].unusedDevicesHelp.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].unusedDevicesHelp.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityChannelButtonBehaviour(
    const Enum<ChannelButtonBehaviour> &v, uint32_t pos)
{
  PropertyAccess<Enum<ChannelButtonBehaviour>> access =
      {
        tr("set channel button behaviour"),
        [this, pos]() {return c.getActivities()[pos].channelButtonBehaviour.get();},
        [this, pos](
            const Enum<ChannelButtonBehaviour> &v) {
              c.getActivities()[pos].channelButtonBehaviour.set(v).setIncluded(Used::YES);
            },
        v,
        Item::ACTIVITY,
        pos };

  return setProperty<Enum<ChannelButtonBehaviour>>(access);
}

bool CmdCatalogue::setActivityControlGroupSoftButtons(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set control group soft buttons"),
        [this, pos]() {return c.getActivities()[pos].controlGroup_SoftButtons.get();},
        [this, pos](
            const bool &v) {
              c.getActivities()[pos].controlGroup_SoftButtons.set(v).setIncluded(Used::YES);
            },
        v,
        Item::ACTIVITY,
        pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityEnableSmartMenu(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set enable smart menu"),
    [this, pos]() {return c.getActivities()[pos].enableSmartMenu.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].enableSmartMenu.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityEnableSmartZoom(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set enable smart zoom"),
    [this, pos]() {return c.getActivities()[pos].enableSmartZoom.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].enableSmartZoom.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityGuideButtonMode(const Enum<GuideButtonMode> &v,
    uint32_t pos)
{
  PropertyAccess<Enum<GuideButtonMode>> access = {
    tr("set guide button mode"),
    [this, pos]() {return c.getActivities()[pos].guideButtonMode.get();},
    [this, pos](const Enum<GuideButtonMode> &v) {
      c.getActivities()[pos].guideButtonMode.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<Enum<GuideButtonMode>>(access);
}

bool CmdCatalogue::setActivityHideModeControl(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set hide mode control"),
    [this, pos]() {return c.getActivities()[pos].hideModeControl.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].hideModeControl.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideModeListen(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set hide mode listen"),
    [this, pos]() {return c.getActivities()[pos].hideModeListen.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].hideModeListen.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideModeNavigate(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set hide mode navigate"),
    [this, pos]() {return c.getActivities()[pos].hideModeNavigate.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].hideModeNavigate.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideModePlay(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set hide mode play"),
    [this, pos]() {return c.getActivities()[pos].hideModePlay.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].hideModePlay.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideModePlayMode(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set hide mode play mode"),
    [this, pos]() {return c.getActivities()[pos].hideModePlayMode.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].hideModePlayMode.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideSurfAllChannels(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set hide surf all channels"),
    [this, pos]() {return c.getActivities()[pos].hideSurfAllChannels.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].hideSurfAllChannels.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideSurfAllShows(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set hide surf all shows"),
    [this, pos]() {return c.getActivities()[pos].hideSurfAllShows.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].hideSurfAllShows.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideSurfFavoriteChannels(bool v, uint32_t pos)
{
  PropertyAccess<bool> access =
      {
        tr("set hide surf favorite channels"),
        [this, pos]() {return c.getActivities()[pos].hideSurfFavoriteChannels.get();},
        [this, pos](
            const bool &v) {
              c.getActivities()[pos].hideSurfFavoriteChannels.set(v).setIncluded(Used::YES);
            },
        v,
        Item::ACTIVITY,
        pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityHideSurfFavoriteShows(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = { tr("set hide surf favorite shows"), [this,
      pos]() {return c.getActivities()[pos].hideSurfFavoriteShows.get();}, [
      this, pos](const bool &v) {
    c.getActivities()[pos].hideSurfFavoriteShows.set(v).setIncluded(Used::YES);
  }, v, Item::ACTIVITY, pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityMaxTvContentDays(int32_t v, uint32_t pos)
{
  PropertyAccess<int32_t> access = {
    tr("set max tv content days"),
    [this, pos]() {return c.getActivities()[pos].maxTvContentDays.get();},
    [this, pos](const int32_t &v) {
      c.getActivities()[pos].maxTvContentDays.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<int32_t>(access);
}

bool CmdCatalogue::setActivityMediaButtonMode(const Enum<MediaButtonMode> &v,
    uint32_t pos)
{
  PropertyAccess<Enum<MediaButtonMode>> access = {
    tr("set media button mode"),
    [this, pos]() {return c.getActivities()[pos].mediaButtonMode.get();},
    [this, pos](const Enum<MediaButtonMode> &v) {
      c.getActivities()[pos].mediaButtonMode.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<Enum<MediaButtonMode>>(access);
}

bool CmdCatalogue::setActivityPlayOnEnter(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set play on enter"),
    [this, pos]() {return c.getActivities()[pos].playOnEnter.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].playOnEnter.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityRetainStop(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set retain stop"),
    [this, pos]() {return c.getActivities()[pos].retainStop.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].retainStop.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityScrollChannelsByPage(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set scroll channels by page"),
    [this, pos]() {return c.getActivities()[pos].scrollChannelsByPage.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].scrollChannelsByPage.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityScrollShowsByPage(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set scroll shows by page"),
    [this, pos]() {return c.getActivities()[pos].scrollShowsByPage.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].scrollShowsByPage.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityStopOnExit(bool v, uint32_t pos)
{
  PropertyAccess<bool> access = {
    tr("set stop on exit"),
    [this, pos]() {return c.getActivities()[pos].stopOnExit.get();},
    [this, pos](const bool &v) {
      c.getActivities()[pos].stopOnExit.set(v).setIncluded(Used::YES);
    },
    v,
    Item::ACTIVITY,
    pos };

  return setProperty<bool>(access);
}

bool CmdCatalogue::setActivityUnknownProperty(
    const data::item::UnknownElement &value, uint32_t pos)
{
  auto *cmd = new SetActivityUnknownPropertyCommand(c, value, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::addActivityChannelCommand(uint32_t activityPos, int chPos)
{
  auto *cmd = new AddActivityChannelCommand(c, activityPos, chPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: channel activity pos %1/%2 already exists, dropped").arg(
            activityPos).arg(chPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeActivityChannelCommand(uint32_t activityPos,
    uint32_t chPos)
{
  auto *cmd = new RemoveActivityChannelCommand(c, activityPos, chPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: button activity pos %1/%2 doesn't exist, dropped").arg(
            activityPos).arg(chPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setActivityChannelStation(const std::string &v,
    uint32_t activityPos, uint32_t chPos)
{
  PropertyAccess<string> access =
      {
        tr("set button activity station name"),
        [this, activityPos, chPos]() {return c.getActivities()[activityPos].getChannels()[chPos].station.get();},
        [this, activityPos, chPos](
            const string &v) {c.getActivities()[activityPos].getChannels()[chPos].station.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_CHANNEL,
        chPos };
  return setProperty<string>(access);
}

bool CmdCatalogue::setActivityChannelNumber(const uint32_t v,
    uint32_t activityPos, uint32_t chPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set button activity channel number"),
        [this, activityPos, chPos]() {return c.getActivities()[activityPos].getChannels()[chPos].channel.get();},
        [this, activityPos, chPos](
            uint32_t v) {c.getActivities()[activityPos].getChannels()[chPos].channel.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_CHANNEL,
        chPos };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setActivityChannelPosition(const uint32_t v,
    uint32_t activityPos, uint32_t chPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set button activity channel button position"),
        [this, activityPos, chPos]() {return c.getActivities()[activityPos].getChannels()[chPos].position.get();},
        [this, activityPos, chPos](
            uint32_t v) {c.getActivities()[activityPos].getChannels()[chPos].position.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_CHANNEL,
        chPos };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setActivityChannelImage(const std::string &v,
    uint32_t activityPos, uint32_t chPos)
{
  PropertyAccess<string> access =
      {
        tr("set button activity channel button image"),
        [this, activityPos, chPos]() {return c.getActivities()[activityPos].getChannels()[chPos].img.get();},
        [this, activityPos, chPos](
            const string &v) {c.getActivities()[activityPos].getChannels()[chPos].img.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_CHANNEL,
        chPos };
  return setProperty<string>(access);
}

bool CmdCatalogue::addActivityButtonCommand(uint32_t activityPos,
    item::ButtonType t, int buttonPos)
{
  auto *cmd = new AddActivityButtonCommand(c, activityPos, t, buttonPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: button in activity  %1 already exists, dropped").arg(
            activityPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeActivityButtonCommand(uint32_t activityPos,
    item::ButtonType t, int buttonPos)
{
  auto *cmd = new RemoveActivityButtonCommand(c, activityPos, t, buttonPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: button in activity %1 doesn't exist, dropped").arg(
            activityPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setActivityButtonAction(const std::string &v,
    uint32_t activityPos, item::ButtonType t, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button activity action"),
        [this, activityPos, t, buttonPos]() {return getActivitiesButton(c, activityPos, t, buttonPos)->action.get();},
        [this, activityPos, t, buttonPos](
            const string &v) {getActivitiesButton(c, activityPos, t, buttonPos)->action.set(v).setIncluded(Used::YES);},
        v,
        toActivityItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setActivityButtonDevice(uint32_t v, uint32_t activityPos,
    item::ButtonType t, int buttonPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set button activity device"),
        [this, activityPos, t, buttonPos]() {return getActivitiesButton(c, activityPos, t, buttonPos)->device.get();},
        [this, activityPos, t, buttonPos](
            const uint32_t &v) {getActivitiesButton(c, activityPos, t, buttonPos)->device.set(v).setIncluded(Used::YES);},
        v,
        toActivityItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setActivityButtonName(const std::string &v,
    uint32_t activityPos, item::ButtonType t, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button activity name"),
        [this, activityPos, t, buttonPos]() {return getActivitiesButton(c, activityPos, t, buttonPos)->name.get();},
        [this, activityPos, t, buttonPos](
            const string &v) {getActivitiesButton(c, activityPos, t, buttonPos)->name.set(v).setIncluded(Used::YES);},
        v,
        toActivityItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setActivityButtonFile(const std::string &v,
    uint32_t activityPos, item::ButtonType t, int buttonPos)
{
  PropertyAccess<string> access =
      {
        tr("set button activity file"),
        [this, activityPos, t, buttonPos]() {return getActivitiesButton(c, activityPos, t, buttonPos)->file.get();},
        [this, activityPos, t, buttonPos](
            const string &v) {getActivitiesButton(c, activityPos, t, buttonPos)->file.set(v).setIncluded(Used::YES);},
        v,
        toActivityItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<string>(access);
}

bool CmdCatalogue::setActivityButtonPosition(const int32_t v,
    uint32_t activityPos, item::ButtonType t, int buttonPos)
{
  PropertyAccess<int32_t> access =
      {
        tr("set button activity position"),
        [this, activityPos, t, buttonPos]() {return getActivitiesButton(c, activityPos, t, buttonPos)->position.get();},
        [this, activityPos, t, buttonPos](
            const int32_t &v) {getActivitiesButton(c, activityPos, t, buttonPos)->position.set(v).setIncluded(Used::YES);},
        v,
        toActivityItem(t),
        static_cast<uint32_t>(buttonPos) };
  return setProperty<int32_t>(access);
}

bool CmdCatalogue::addActivityActionCommand(uint32_t activityPos,
    item::ActivityAction t, int actionPos)
{
  auto *cmd = new AddActivityActionCommand(c, activityPos, t, actionPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: action activity pos %1/%2/%3 already exists, dropped").arg(
            activityPos).arg((int) t).arg(actionPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removActivityActionCommand(uint32_t activityPos,
    item::ActivityAction t, int actionPos)
{
  auto *cmd = new RemoveActivityActionCommand(c, activityPos, t, actionPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: action activity pos %1/%2/%3 doesn't exist, dropped").arg(
            activityPos).arg((int) t).arg(actionPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::addActivityActionSequenceCommand(uint32_t activityPos,
    item::ActivityAction t, int actionPos, int seqPos)
{
  auto *cmd = new AddActionSequenceCommand(c, activityPos, t, actionPos,
      seqPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: activity act-seq pos %1/%2b/%3/%4 failed, dropped").arg(
            activityPos).arg((int) t).arg(actionPos).arg(seqPos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeActivityActionSequenceCommand(uint32_t activityPos,
    item::ActivityAction t, int actionPos, uint32_t seqPos)
{
  auto *cmd = new RemoveDeviceActionSequenceCommand(c, activityPos, t,
      actionPos, seqPos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: activity act-seq pos %1/%2b/%3/%4 doesn't exist, dropped").arg(
            activityPos).arg((int) t).arg(actionPos).arg(seqPos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setActivityActionSequenceOp(const Enum<Operation> &v,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    uint32_t seqPos)
{
  PropertyAccess<Enum<Operation>> access =
      {
        tr("set activity action sequence opcode"),
        [this, activityPos, t, actionPos, seqPos]() {return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].opcode.get();},
        [this, activityPos, t, actionPos, seqPos](
            const Enum<Operation> &v) {getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].opcode.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_ACTION,
        activityPos };
  return setProperty<Enum<Operation>>(access);
}

bool CmdCatalogue::setActivityActionSequenceCmd(const std::string &v,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set activity action cmd"),
        [this, activityPos, t, actionPos, seqPos]() {return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].cmd.get();},
        [this, activityPos, t, actionPos, seqPos](
            const string &v) {getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].cmd.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_ACTION,
        activityPos };
  return setProperty<string>(access);
}

bool CmdCatalogue::setActivityActionSequenceDeviceId(uint32_t v,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    uint32_t seqPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set activity action controlled device id"),
        [this, activityPos, t, actionPos, seqPos]() {return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].deviceId.get();},
        [this, activityPos, t, actionPos, seqPos](
            const uint32_t &v) {getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].deviceId.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_ACTION,
        activityPos };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setActivityActionSequenceDelayMs(uint32_t v,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    uint32_t seqPos)
{
  PropertyAccess<uint32_t> access =
      {
        tr("set activity action delay"),
        [this, activityPos, t, actionPos, seqPos]() {return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].delayMs.get();},
        [this, activityPos, t, actionPos, seqPos](
            const uint32_t &v) {getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].delayMs.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_ACTION,
        activityPos };
  return setProperty<uint32_t>(access);
}

bool CmdCatalogue::setActivityActionSequenceStateName(
    const Enum<StateMachineDeviceType> &v, uint32_t activityPos,
    item::ActivityAction t, int actionPos, uint32_t seqPos)
{
  PropertyAccess<Enum<StateMachineDeviceType>> access =
      {
        tr("set activity action state name"),
        [this, activityPos, t, actionPos, seqPos]() {return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].stateName.get();},
        [this, activityPos, t, actionPos, seqPos](
            const Enum<StateMachineDeviceType> &v) {getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].stateName.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_ACTION,
        activityPos };
  return setProperty<Enum<StateMachineDeviceType>>(access);
}

bool CmdCatalogue::setActivityActionSequenceStateValue(const std::string &v,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    uint32_t seqPos)
{
  PropertyAccess<string> access =
      {
        tr("set activity action state value"),
        [this, activityPos, t, actionPos, seqPos]() {return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].stateValue.get();},
        [this, activityPos, t, actionPos, seqPos](
            const string &v) {getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].stateValue.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_ACTION,
        activityPos };
  return setProperty<string>(access);
}

bool CmdCatalogue::setActivityActionSequenceMod(const Enum<Modifier> &v,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    uint32_t seqPos)
{
  PropertyAccess<Enum<Modifier>> access =
      {
        tr("set activity action state modifier"),
        [this, activityPos, t, actionPos, seqPos]() {return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].mod.get();},
        [this, activityPos, t, actionPos, seqPos](
            const Enum<Modifier> &v) {getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].mod.set(v).setIncluded(Used::YES);},
        v,
        Item::ACTIVITY_ACTION,
        activityPos };
  return setProperty<Enum<Modifier>>(access);
}

bool CmdCatalogue::setActivityActionUnknownParam(
    const data::item::UnknownElement &v, uint32_t activityPos,
    item::ActivityAction t, int actionPos, uint32_t seqPos)
{
  auto *cmd = new SetActivityActionUnknownParamCommand(c, v, activityPos, t,
      actionPos, seqPos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setActivityRoleCommand(uint32_t activityPos,
    item::Role &role, int rolePos, bool overwrite)
{
  auto *cmd = new SetRoleCommand(c, activityPos, role, rolePos, overwrite);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: set role in activity %1 failed, dropped").arg(activityPos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeActivityRoleCommand(uint32_t activityPos,
    uint32_t rolePos)
{
  auto *cmd = new RemoveRoleCommand(c, activityPos, rolePos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: remove role from activity %1 failed, dropped").arg(
            activityPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeActivityRoleCommandById(uint32_t activityPos,
    uint32_t deviceId)
{
  auto *cmd = RemoveRoleCommand::fromId(c, activityPos, deviceId);
  if (cmd != nullptr) {
    if (cmd->valid() == true) {
      connectCommand(cmd);
      undo.push(cmd);
      return true;
    }
    delete cmd;
  }
  emit writeLog(LogLevel::Warning,
      tr("modify: remove role from activity %1 failed, dropped").arg(
          activityPos), ContentType::PlainText);
  return false;
}

bool CmdCatalogue::addActivityPowerOnDevicesCommand(uint32_t activityPos,
    uint32_t id, int devicePos)
{
  auto *cmd = new AddPowerOnDevice(c, activityPos, id, devicePos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: add power on to activity %1 failed, dropped").arg(
            activityPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeActivityPowerOnDevicesCommand(uint32_t activityPos,
    uint32_t devicePos)
{
  auto *cmd = new RemovePowerOnDevice(c, activityPos, devicePos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: remove power on from activity %1 failed, dropped").arg(
            activityPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeActivityPowerOnDevicesCommandById(uint32_t activityPos,
    uint32_t deviceId)
{
  auto *cmd = RemovePowerOnDevice::fromId(c, activityPos, deviceId);
  if (cmd != nullptr) {
    if (cmd->valid() == true) {
      connectCommand(cmd);
      undo.push(cmd);
      return true;
    }
    delete cmd;
  }
  emit writeLog(LogLevel::Warning,
      tr("modify: remove power on from activity %1 failed, dropped").arg(
          activityPos), ContentType::PlainText);
  return false;
}

bool CmdCatalogue::addActivityPowerOffDevicesCommand(uint32_t activityPos,
    uint32_t id, int devicePos)
{
  auto *cmd = new AddPowerOffDevice(c, activityPos, id, devicePos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: add power off to activity %1 failed, dropped").arg(
            activityPos), ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::removeActivityPowerOffDevicesCommand(uint32_t activityPos,
    uint32_t devicePos)
{
  auto *cmd = new RemovePowerOffDevice(c, activityPos, devicePos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: remove power off from activity %1 failed, dropped").arg(
            activityPos), ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::removeActivityPowerOffDevicesCommandById(
    uint32_t activityPos, uint32_t deviceId)
{
  auto *cmd = RemovePowerOffDevice::fromId(c, activityPos, deviceId);
  if (cmd != nullptr) {
    if (cmd->valid() == true) {
      connectCommand(cmd);
      undo.push(cmd);
      return true;
    }
    delete cmd;
  }
  emit writeLog(LogLevel::Warning,
      tr("modify: remove power off from activity %1 failed, dropped").arg(
          activityPos), ContentType::PlainText);
  return false;
}

bool CmdCatalogue::setIrProtoLib(const binary::irProto::File &file)
{
  auto *cmd = new SetIrProtoLibCommand(c, file);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

int CmdCatalogue::appendIrProtoLibItem(const Enum<CodeType> &t)
{
  int index = -1;

  auto *cmd = new AppendIrProtoLibItemCommand(c, t);
  auto ret = cmd->valid();
  if (ret == true) {
    index = cmd->index();
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: append ir proto to lib failed, dropped"),
        ContentType::PlainText);
    delete cmd;
  }
  return index;
}

void CmdCatalogue::connectCommand(BaseCommand *cmd)
{
  //connect signals. not all commands actually use all signals!

  // @formatter:off
  connect(cmd, &BaseCommand::writeLog, this, &CmdCatalogue::writeLog);
  connect(cmd, &BaseCommand::writeMsg, this, &CmdCatalogue::writeMsg);
  connect(cmd, &BaseCommand::itemChanged, this, &CmdCatalogue::itemChanged);
  connect(cmd, &BaseCommand::itemAboutToBeAdded, this, &CmdCatalogue::itemAboutToBeAdded);
  connect(cmd, &BaseCommand::itemAdded, this, &CmdCatalogue::itemAdded);
  connect(cmd, &BaseCommand::itemAboutToBeRemoved, this, &CmdCatalogue::itemAboutToBeRemoved);
  connect(cmd, &BaseCommand::itemRemoved, this, &CmdCatalogue::itemRemoved);
  connect(cmd, &BaseCommand::dirtyChanged, this, &CmdCatalogue::dirtyChanged);
// @formatter:on
}

}
}
