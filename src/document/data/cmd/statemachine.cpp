// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>

#include "lib/qtHelpers.h"
#include "state.h"
#include "statemachine.h"

using namespace std;

namespace document
{
namespace data
{

EditStateMachineCommand::EditStateMachineCommand(ConfigData &c,
    uint32_t devicePos, const QString &desc, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Statemachine (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos)
{
}

bool EditStateMachineCommand::valid() const
{
  return isValid;
}

bool EditStateMachineCommand::checkRemove(const item::StateMachine &oldSm,
    const item::StateMachine &sm)
{
  QStringList oldStates;
  QStringList newStates;
  QStringList diff;

  //fixme can we use signal/slot for this? the current implementation
  //requires all dependencies to be hard-coded.

  //check if machine was changed/removed
  if (oldSm.smType.get().getValue() != sm.smType.get().getValue()) {
    return checkRemove(oldSm.smType.get().getValue());
  }

  //check if states where changed/removed. we don't care if a state was moved
  //from one list to the other, only removing is relevant
  oldStates = lib::toQStringList(oldSm.discrete.states)
      + lib::toQStringList(oldSm.relative.states);
  newStates = lib::toQStringList(sm.discrete.states)
      + lib::toQStringList(sm.relative.states);
  for (const QString &state : oldStates) {
    if (!newStates.contains(state)) {
      diff.append(state);
    }
  }
  for (const QString &state : diff) {
    auto ret = EditStateCommand::checkRemove(c, devicePos,
        oldSm.smType.get().getValue(), state);
    if (ret != true) {
      return false;
    }
  }
  return true;
}

bool EditStateMachineCommand::checkRemove(StateMachineDeviceType type)
{
  int i;
  bool reject = false;

  if (type == StateMachineDeviceType::Unknown) {
    return true;
  }

  //check if we self-reference the machine
  auto &device = c.getDevices()[devicePos];
  for (const auto &sm : device.getStateMachines()) {
    if (sm.startAction.has_value()) {
      reject |= checkDeviceAction(*(sm.startAction), device.getId(), type);
    }
    if (sm.finishAction.has_value()) {
      reject |= checkDeviceAction(*(sm.finishAction), device.getId(), type);
    }
    for (i = 0; i < sm.discrete.enterStateAction.size(); i++) {
      reject |= checkDeviceAction(sm.discrete.enterStateAction[i],
          device.getId(), type);
    }
    if (sm.relative.resetAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.resetAction), device.getId(),
          type);
    }
    if (sm.relative.nextStateAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.nextStateAction),
          device.getId(), type);
    }
    if (sm.relative.prevStateAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.prevStateAction),
          device.getId(), type);
    }
    if (reject) {
      QMessageBox::warning(nullptr, tr("Control is used"),
          tr("This control is currently used by the control \"%2\" "
              "within the same device. It can't be modified or deleted, "
              "without removing it there first.").arg(
              sm.smType.get().getQString()), QMessageBox::Ok);
      return false;
    }
  }

  //property overrides activity settings
  if ((type == StateMachineDeviceType::Power)
      && (device.alwaysOn.get() || device.manualPower.get())) {
    return true;
  }

  //check if the machine is referenced in activities
  auto &activities = c.getActivities();
  auto deviceId = device.getId();
  for (i = 0; i < activities.size(); i++) {
    if (type == StateMachineDeviceType::Power) {
      for (const auto &on : activities[i].getPowerOnDevices()) {
        if (deviceId == on) {
          reject = true;
        }
      }
    }
    //todo aus power-off rausloeschen?
    for (const auto &a : activities[i].getEnterActions()) {
      reject |= checkDeviceAction(a, device.getId(), type);
    }
    for (const auto &a : activities[i].getLeaveActions()) {
      reject |= checkDeviceAction(a, device.getId(), type);
    }
    if (reject) {
      QMessageBox::warning(nullptr, tr("Control is used"),
          tr("This control is currently used in the activity \"%1\"."
              "It can't be modified or deleted, "
              "without removing it there first.").arg(
              qstr(activities[i].label.get())), QMessageBox::Ok);
      return false;
    }
  }
  return true;
}

bool EditStateMachineCommand::checkDeviceAction(const item::DeviceAction &d,
    uint32_t deviceId, StateMachineDeviceType type)
{
  for (const auto &s : d.sequence) {
    if ((s.deviceId.isIncluded() == Used::YES)
        && (s.deviceId.get() != deviceId)) {
      continue;
    }
    if (s.stateName.get().getValue() == type) {
      return true;
    }
  }
  return false;
}

AddStatemachineCommand::AddStatemachineCommand(ConfigData &c,
    uint32_t devicePos, int smPos, QUndoCommand *parent) :
    EditStateMachineCommand(c, devicePos,
        QObject::tr("Add Statemachine (to device: %1)").arg(devicePos), parent)
{
  uint32_t smCount;

  if (devicePos >= c.getDevices().size()) {
    return;
  }
  smCount = c.getDevices()[devicePos].getStateMachines().size();
  if (smPos < 0) {
    //append
    smPos = smCount;
  }
  if (smPos > smCount) {
    return;
  }
  this->smPos = smPos;
  isValid = true;
}

void AddStatemachineCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_STATEMACHINE, smPos);
  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.insert(sms.begin() + smPos, item::StateMachine());
  emit itemAdded(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void AddStatemachineCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_STATEMACHINE, smPos);
  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.erase(sms.begin() + smPos);
  emit itemRemoved(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

SetStatemachineCommand::SetStatemachineCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, const item::StateMachine &sm,
    QUndoCommand *parent) :
    EditStateMachineCommand(c, devicePos,
        QObject::tr("Update Statemachine (in device: %1)").arg(devicePos),
        parent), smPos(smPos), sm(sm)
{
  try {
    oldSm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    isValid = checkRemove(oldSm, sm);
  } catch (std::out_of_range&) {
  }
}

void SetStatemachineCommand::redo()
{
  if (!isValid) {
    return;
  }

  c.getDevices()[devicePos].getStateMachines()[smPos] = sm;

  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void SetStatemachineCommand::undo()
{
  if (!isValid) {
    return;
  }

  c.getDevices()[devicePos].getStateMachines()[smPos] = oldSm;

  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

RemoveStatemachineCommand::RemoveStatemachineCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, QUndoCommand *parent) :
    EditStateMachineCommand(c, devicePos,
        QObject::tr("Remove Statemachine (Pos: %1)").arg(devicePos), parent), smPos(
        smPos)
{
  uint32_t smCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }
  smCount = c.getDevices()[devicePos].getStateMachines().size();
  if (smPos >= smCount) {
    return;
  }
  sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  isValid = checkRemove(sm, item::StateMachine());
}

void RemoveStatemachineCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_STATEMACHINE, smPos);
  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.erase(sms.begin() + smPos);
  emit itemRemoved(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void RemoveStatemachineCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_STATEMACHINE, smPos);
  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.insert(sms.begin() + smPos, sm);
  emit itemAdded(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

}
}
