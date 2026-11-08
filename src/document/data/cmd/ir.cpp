// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>

#include "button.h"
#include "ir.h"

using namespace std;

namespace document
{
namespace data
{


SetIrCommand::SetIrCommand(ConfigData &c, uint32_t devicePos,
    item::ProtoCommand &cmd, int cmdPos, bool overwrite, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set Proto Cmd (device: %1)").arg(devicePos),
        parent), isValid(false), overwrite(overwrite), c(c), devicePos(
        devicePos), cmdPos(cmdPos), proto(cmd)
{
  if (devicePos >= c.getDevices().size()) {
    return;
  }
  if (cmd.name.get().empty()) {
    return;
  }
  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  if (!overwrite && cmds.nameExists(cmd.name.get())) {
    emit writeMsg(tr("Command %1 already exists").arg(qstr(cmd.name.get())));
    return;
  }

  uint32_t cmdCount = cmds.getProtoCommands().size();

  if (overwrite) {
    if ((cmdPos < 0) || (static_cast<uint32_t>(cmdPos) >= cmdCount)) {
      return;
    }
    prevProto = cmds.getProtoCommands()[cmdPos];
  } else {
    if (cmdPos < 0) {
      cmdPos = static_cast<int>(cmdCount);
    }
    if (static_cast<uint32_t>(cmdPos) > cmdCount) {
      return;
    }
  }

  this->cmdPos = cmdPos;
  isValid = true;
}

SetIrCommand::SetIrCommand(ConfigData &c, uint32_t devicePos,
    item::RawCommand &cmd, int cmdPos, bool overwrite, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set Raw Cmd (device: %1)").arg(devicePos), parent), overwrite(
        overwrite), c(c), devicePos(devicePos), raw(cmd)
{
  if (devicePos >= c.getDevices().size()) {
    return;
  }
  if (cmd.name.get().empty()) {
    return;
  }
  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  if (!overwrite && cmds.nameExists(cmd.name.get())) {
    emit writeMsg(tr("Command %1 already exists").arg(qstr(cmd.name.get())));
    return;
  }

  uint32_t cmdCount = cmds.getRawCommands().size();

  if (overwrite) {
    if ((cmdPos < 0) || (static_cast<uint32_t>(cmdPos) >= cmdCount)) {
      return;
    }
    prevRaw = cmds.getRawCommands()[cmdPos];
  } else {
    if (cmdPos < 0) {
      cmdPos = static_cast<int>(cmdCount);
    }
    if (static_cast<uint32_t>(cmdPos) > cmdCount) {
      return;
    }
  }

  this->cmdPos = cmdPos;
  isValid = true;
}

void SetIrCommand::redo()
{
  if (!isValid) {
    return;
  }

  if (!overwrite) {
    emit itemAboutToBeAdded(Item::DEVICE_IR_DATA, cmdPos);
  }

  if (proto.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
    if (overwrite) {
      cmds[cmdPos] = proto.value();
      updateActions(prevProto->name.get(), proto->name.get());
    } else {
      cmds.insert(cmds.begin() + cmdPos, proto.value());
    }
  } else if (raw.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
    if (overwrite) {
      cmds[cmdPos] = raw.value();
      updateActions(prevRaw->name.get(), raw->name.get());
    } else {
      cmds.insert(cmds.begin() + cmdPos, raw.value());
    }
  }

  if (overwrite) {
    emit itemChanged(Item::DEVICE_IR_DATA, cmdPos);
  } else {
    emit itemAdded(Item::DEVICE_IR_DATA, cmdPos);
  }
  emit dirtyChanged(true);
}

void SetIrCommand::undo()
{
  if (!isValid) {
    return;
  }

  if (!overwrite) {
    emit itemAboutToBeRemoved(Item::DEVICE_IR_DATA, cmdPos);
  }

  if (proto.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
    if (overwrite) {
      cmds[cmdPos] = prevProto.value();
      updateActions(proto->name.get(), prevProto->name.get());
    } else {
      cmds.erase(cmds.begin() + cmdPos);
    }
  } else if (raw.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
    if (overwrite) {
      cmds[cmdPos] = prevRaw.value();
      updateActions(raw->name.get(), prevRaw->name.get());
    } else {
      cmds.erase(cmds.begin() + cmdPos);
    }
  }

  if (overwrite) {
    emit itemChanged(Item::DEVICE_IR_DATA, cmdPos);
  } else {
    emit itemRemoved(Item::DEVICE_IR_DATA, cmdPos);
  }
  emit dirtyChanged(true);
}

bool SetIrCommand::valid() const
{
  return isValid;
}

void SetIrCommand::updateActions(const string &oldAction,
    const string &newAction)
{
  int i;
  int j;

  if (oldAction == newAction) {
    return;
  }

  //fixme can we use signal/slot for this? the current implementation
  //requires all dependencies to be hard-coded.
  //we could also assign a uid to each IR command.

  //update the name within our device
  auto &device = c.getDevices()[devicePos];
  auto &hardButtons = device.getHardButtons();
  for (i = 0; i < hardButtons.size(); i++) {
    auto &button = hardButtons[i];
    if (button.action.get() == oldAction) {
      button.action.set(newAction);
      emit itemChanged(Item::DEVICE_HARD_BUTTON, i);
    }
  }
  auto &softButtons = device.getSoftButtons();
  for (i = 0; i < softButtons.size(); i++) {
    auto &button = softButtons[i];
    if (button.action.get() == oldAction) {
      button.action.set(newAction);
      emit itemChanged(Item::DEVICE_SOFT_BUTTON, i);
    }
  }
  auto &stateMachines = device.getStateMachines();
  for (i = 0; i < stateMachines.size(); i++) {
    auto &sm = stateMachines[i];
    if (sm.startAction.has_value()) {
      updateDeviceAction(*(sm.startAction), oldAction, newAction,
          Item::DEVICE_STATEMACHINE, i);
    }
    if (sm.finishAction.has_value()) {
      updateDeviceAction(*(sm.finishAction), oldAction, newAction,
          Item::DEVICE_STATEMACHINE, i);
    }
    for (j = 0; j < sm.discrete.enterStateAction.size(); j++) {
      updateDeviceAction(sm.discrete.enterStateAction[j], oldAction, newAction,
          Item::DEVICE_STATEMACHINE, i);
    }
    if (sm.relative.resetAction.has_value()) {
      updateDeviceAction(*(sm.relative.resetAction), oldAction, newAction,
          Item::DEVICE_STATEMACHINE, i);
    }
    if (sm.relative.nextStateAction.has_value()) {
      updateDeviceAction(*(sm.relative.nextStateAction), oldAction, newAction,
          Item::DEVICE_STATEMACHINE, i);
    }
    if (sm.relative.prevStateAction.has_value()) {
      updateDeviceAction(*(sm.relative.prevStateAction), oldAction, newAction,
          Item::DEVICE_STATEMACHINE, i);
    }
  }
  auto numpad = device.getNumpad();
  if (numpad.has_value()) {
    if (numpad->first.has_value()) {
      for (i = 0; i < numpad->first->size(); i++) {
        updateDeviceAction((*numpad->first)[i], oldAction, newAction,
            Item::DEVICE_NUMPAD, i);
      }
    }
    if (numpad->middle.has_value()) {
      for (i = 0; i < numpad->middle->size(); i++) {
        updateDeviceAction((*numpad->middle)[i], oldAction, newAction,
            Item::DEVICE_NUMPAD, i);
      }
    }
    if (numpad->last.has_value()) {
      for (i = 0; i < numpad->last->size(); i++) {
        updateDeviceAction((*numpad->last)[i], oldAction, newAction,
            Item::DEVICE_NUMPAD, i);
      }
    }
    if (numpad->start.has_value()) {
      updateDeviceAction(*(numpad->start), oldAction, newAction,
          Item::DEVICE_NUMPAD, 0);
    }
    if (numpad->greaterTen.has_value()) {
      updateDeviceAction(*(numpad->greaterTen), oldAction, newAction,
          Item::DEVICE_NUMPAD, 0);
    }
    if (numpad->greaterHundred.has_value()) {
      updateDeviceAction(*(numpad->greaterHundred), oldAction, newAction,
          Item::DEVICE_NUMPAD, 0);
    }
    if (numpad->finish.has_value()) {
      updateDeviceAction(*(numpad->finish), oldAction, newAction,
          Item::DEVICE_NUMPAD, 0);
    }
  }

  //update the name in referencing activities
  auto &activities = c.getActivities();
  auto deviceId = device.getId();
  for (i = 0; i < activities.size(); i++) {
    auto &hardButtons = activities[i].getHardButtons();
    for (j = 0; j < hardButtons.size(); j++) {
      auto &button = hardButtons[j];
      if ((button.device.get() == deviceId)
          && (button.action.get() == oldAction)) {
        button.action.set(newAction);
        emit itemChanged(Item::ACTIVITY_HARD_BUTTON, j);
      }
    }
    auto &softButtons = activities[i].getSoftButtons();
    for (j = 0; j < softButtons.size(); j++) {
      auto &button = softButtons[j];
      if ((button.device.get() == deviceId)
          && (button.action.get() == oldAction)) {
        button.action.set(newAction);
        emit itemChanged(Item::ACTIVITY_SOFT_BUTTON, j);
      }
    }
  }
}

void SetIrCommand::updateDeviceAction(item::DeviceAction &d,
    const string &oldAction, const string &newAction, Item event, int eventPos)
{
  for (auto &s : d.sequence) {
    if (s.cmd.get() == oldAction) {
      s.cmd.set(newAction);
      emit itemChanged(event, eventPos);
    }
  }
}



RemoveIrCommand::RemoveIrCommand(ConfigData &c, uint32_t devicePos,
    uint32_t cmdPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Cmd (Pos: %1)").arg(devicePos), parent), c(
        c), devicePos(devicePos), cmdPos(cmdPos)
{
}

bool RemoveIrCommand::valid() const
{
  return isValid;
}

bool RemoveIrCommand::checkRemove(const std::string &action)
{
  int i;
  int j;
  bool query = false;
  bool reject = false;

  //fixme can we use signal/slot for this? the current implementation
  //requires all dependencies to be hard-coded.
  //we could also assign a uid to each IR command.

  //check the name within our device
  auto &device = c.getDevices()[devicePos];
  auto &hardButtons = device.getHardButtons();
  for (i = 0; i < hardButtons.size(); i++) {
    auto &button = hardButtons[i];
    if (button.action.get() == action) {
      query = true;
    }
  }
  auto &softButtons = device.getSoftButtons();
  for (i = 0; i < softButtons.size(); i++) {
    auto &button = softButtons[i];
    if (button.action.get() == action) {
      query = true;
    }
  }
  auto &stateMachines = device.getStateMachines();
  for (i = 0; i < stateMachines.size(); i++) {
    auto &sm = stateMachines[i];
    if (sm.startAction.has_value()) {
      reject |= checkDeviceAction(*(sm.startAction), action);
    }
    if (sm.finishAction.has_value()) {
      reject |= checkDeviceAction(*(sm.finishAction), action);
    }
    for (j = 0; j < sm.discrete.enterStateAction.size(); j++) {
      reject |= checkDeviceAction(sm.discrete.enterStateAction[j], action);
    }
    if (sm.relative.resetAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.resetAction), action);
    }
    if (sm.relative.nextStateAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.nextStateAction), action);
    }
    if (sm.relative.prevStateAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.prevStateAction), action);
    }
  }
  auto numpad = device.getNumpad();
  if (numpad.has_value()) {
    if (numpad->first.has_value()) {
      for (i = 0; i < numpad->first->size(); i++) {
        reject |= checkDeviceAction((*numpad->first)[i], action);
      }
    }
    if (numpad->middle.has_value()) {
      for (i = 0; i < numpad->middle->size(); i++) {
        reject |= checkDeviceAction((*numpad->middle)[i], action);
      }
    }
    if (numpad->last.has_value()) {
      for (i = 0; i < numpad->last->size(); i++) {
        reject |= checkDeviceAction((*numpad->last)[i], action);
      }
    }
    if (numpad->start.has_value()) {
      reject |= checkDeviceAction(*(numpad->start), action);
    }
    if (numpad->greaterTen.has_value()) {
      reject |= checkDeviceAction(*(numpad->greaterTen), action);
    }
    if (numpad->greaterHundred.has_value()) {
      reject |= checkDeviceAction(*(numpad->greaterHundred), action);
    }
    if (numpad->finish.has_value()) {
      reject |= checkDeviceAction(*(numpad->finish), action);
    }
  }

  //check the name in referencing activities
  auto &activities = c.getActivities();
  auto deviceId = device.getId();
  for (i = 0; i < activities.size(); i++) {
    auto &hardButtons = activities[i].getHardButtons();
    for (j = 0; j < hardButtons.size(); j++) {
      auto &button = hardButtons[j];
      if ((button.device.get() == deviceId)
          && (button.action.get() == action)) {
        query = true;
      }
    }
    auto &softButtons = activities[i].getSoftButtons();
    for (j = 0; j < softButtons.size(); j++) {
      auto &button = softButtons[j];
      if ((button.device.get() == deviceId)
          && (button.action.get() == action)) {
        query = true;
      }
    }
  }
  if (reject) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(nullptr, tr("Command is used"),
        tr("This command is used to control the device and can't "
            "be deleted (e.g power, numbers, input selection)."
            "Delete it there first.\n\n"
            "Ignoring will leave your config in a broken state. Only use "
            "\"Ignore\" for re-adding it in the other IR commands table."),
        QMessageBox::Ok | QMessageBox::Ignore);
    if (reply == QMessageBox::Ignore) {
      return true;
    }
    return false;
  }
  if (query) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, tr("Delete buttons"),
        tr("This Command is used inside buttons. This will also "
            "delete those.\n\n"
            "Ignoring will leave your config in a broken state. Only use "
            "\"Ignore\" for re-adding it in the other IR commands table."),
        QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Ignore);
    if (reply == QMessageBox::Ok) {
      return doRemove(action);
    } else if (reply == QMessageBox::Ignore) {
      return true;
    }
    return false;
  }
  return true;
}

bool RemoveIrCommand::doRemove(const std::string &action)
{
  int i;
  int j;
  bool ret = true;

  //check the name within our device
  auto &device = c.getDevices()[devicePos];
  auto &hardButtons = device.getHardButtons();
  for (i = hardButtons.size() - 1; i >= 0; i--) {
    auto &button = hardButtons[i];
    if (button.action.get() == action) {
      auto *cmd = new RemoveDeviceButtonCommand(c, devicePos,
          item::ButtonType::Hard, i, this);
      cmd->connectCommand(this);
      ret &= cmd->valid();
    }
  }
  auto &softButtons = device.getSoftButtons();
  for (i = softButtons.size() - 1; i >= 0; i--) {
    auto &button = softButtons[i];
    if (button.action.get() == action) {
      auto *cmd = new RemoveDeviceButtonCommand(c, devicePos,
          item::ButtonType::Soft, i, this);
      cmd->connectCommand(this);
      ret &= cmd->valid();
    }
  }

  //check the name in referencing activities
  auto &activities = c.getActivities();
  auto deviceId = device.getId();
  for (i = 0; i < activities.size(); i++) {
    auto &hardButtons = activities[i].getHardButtons();
    for (j = hardButtons.size() - 1; j >= 0; j--) {
      auto &button = hardButtons[j];
      if ((button.device.get() == deviceId)
          && (button.action.get() == action)) {
        auto *cmd = new RemoveActivityButtonCommand(c, i,
            item::ButtonType::Hard, j, this);
        cmd->connectCommand(this);
        ret &= cmd->valid();
      }
    }
    auto &softButtons = activities[i].getSoftButtons();
    for (j = softButtons.size() - 1; j >= 0; j--) {
      auto &button = softButtons[j];
      if ((button.device.get() == deviceId)
          && (button.action.get() == action)) {
        auto *cmd = new RemoveActivityButtonCommand(c, i,
            item::ButtonType::Soft, j, this);
        cmd->connectCommand(this);
        ret &= cmd->valid();
      }
    }
  }
  return ret;
}

bool RemoveIrCommand::checkDeviceAction(item::DeviceAction &d,
    const std::string &action)
{
  for (auto &s : d.sequence) {
    if (s.cmd.get() == action) {
      return true;
    }
  }
  return false;
}

RemoveIrProtoCommand::RemoveIrProtoCommand(ConfigData &c, uint32_t devicePos,
    uint32_t cmdPos, QUndoCommand *parent) :
    RemoveIrCommand(c, devicePos, cmdPos, parent)
{
  uint32_t cmdCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }

  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  cmdCount = cmds.getProtoCommands().size();
  if (cmdPos >= cmdCount) {
    return;
  }
  proto = cmds.getProtoCommands()[cmdPos];
  isValid = checkRemove(proto.name.get());
}

void RemoveIrProtoCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_IR_DATA, cmdPos);
  QUndoCommand::redo();
  auto &pcmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
  pcmds.erase(pcmds.begin() + cmdPos);
  emit itemRemoved(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

void RemoveIrProtoCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_IR_DATA, cmdPos);
  auto &pcmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
  pcmds.insert(pcmds.begin() + cmdPos, proto);
  QUndoCommand::undo();
  emit itemAdded(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

RemoveIrRawCommand::RemoveIrRawCommand(ConfigData &c, uint32_t devicePos,
    uint32_t cmdPos, QUndoCommand *parent) :
    RemoveIrCommand(c, devicePos, cmdPos, parent)
{
  uint32_t cmdCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }

  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  cmdCount = cmds.getRawCommands().size();
  if (cmdPos >= cmdCount) {
    return;
  }
  raw = cmds.getRawCommands()[cmdPos];
  isValid = checkRemove(raw.name.get());
}

void RemoveIrRawCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_IR_DATA, cmdPos);
  QUndoCommand::redo();
  auto &rcmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
  rcmds.erase(rcmds.begin() + cmdPos);
  emit itemRemoved(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

void RemoveIrRawCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_IR_DATA, cmdPos);
  auto &rcmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
  rcmds.insert(rcmds.begin() + cmdPos, raw);
  QUndoCommand::undo();
  emit itemAdded(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

}
}
