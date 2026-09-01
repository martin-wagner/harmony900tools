// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>

#include "setActionData.h"
#include "state.h"

using namespace std;

namespace document
{
namespace data
{

EditStateCommand::EditStateCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateMachineType t, const QString &desc,
    QUndoCommand *parent) :
    BaseCommand(desc), c(c), devicePos(devicePos), smPos(smPos), t(t)
{
}

bool EditStateCommand::valid() const
{
  return isValid;
}

bool EditStateCommand::checkRemove(ConfigData &c, uint32_t devicePos,
    StateMachineDeviceType type, const QString &state)
{

  int i;
  bool reject = false;

  if (type == StateMachineDeviceType::Unknown) {
    return true;
  }

  //check power on/off. those are mandatory for power state machine
  if (type == StateMachineDeviceType::Power) {
    if ((state == "On") || (state == "Off")) {
      QMessageBox::warning(nullptr, tr("Control power"),
          tr("For power control, you can't remove or rename "
              "\"On\" and \"Off\"."), QMessageBox::Ok);
      return false;
    }
  }

  //check if we self-reference the state
  auto &device = c.getDevices()[devicePos];
  for (const auto &sm : device.getStateMachines()) {
    if (sm.startAction.has_value()) {
      reject |= checkDeviceAction(*(sm.startAction), device.getId(), type,
          state);
    }
    if (sm.finishAction.has_value()) {
      reject |= checkDeviceAction(*(sm.finishAction), device.getId(), type,
          state);
    }
    for (i = 0; i < sm.discrete.enterStateAction.size(); i++) {
      reject |= checkDeviceAction(sm.discrete.enterStateAction[i],
          device.getId(), type, state);
    }
    if (sm.relative.resetAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.resetAction), device.getId(),
          type, state);
    }
    if (sm.relative.nextStateAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.nextStateAction),
          device.getId(), type, state);
    }
    if (sm.relative.prevStateAction.has_value()) {
      reject |= checkDeviceAction(*(sm.relative.prevStateAction),
          device.getId(), type, state);
    }
    if (reject) {
      QMessageBox::warning(nullptr, tr("Control state is used"),
          tr("The control state \"%1\" is currently used by the control \"%2\" "
              "within the same device. It can't be modified or deleted, "
              "without removing it there first.").arg(state).arg(
              sm.smType.get().getQString()), QMessageBox::Ok);
      return false;
    }
  }

  //check if the state is referenced in activities
  auto &activities = c.getActivities();
  for (i = 0; i < activities.size(); i++) {
    for (const auto &a : activities[i].getEnterActions()) {
      reject |= checkDeviceAction(a, device.getId(), type, state);
    }
    for (const auto &a : activities[i].getLeaveActions()) {
      reject |= checkDeviceAction(a, device.getId(), type, state);
    }
    if (reject) {
      QMessageBox::warning(nullptr, tr("Control state is used"),
          tr("The control state \"%1\" is currently used in the "
              "activity \"%2\". "
              "It can't be modified or deleted, "
              "without removing it there first.").arg(state).arg(
              qstr(activities[i].label.get())), QMessageBox::Ok);
      return false;
    }
  }
  return true;
}

bool EditStateCommand::checkRemove(StateMachineDeviceType type,
    const QString &state)
{
  return checkRemove(c, devicePos, type, state);
}

bool EditStateCommand::checkDeviceAction(const item::DeviceAction &a,
    uint32_t deviceId, StateMachineDeviceType type, const QString &state)
{
  for (const auto &s : a.sequence) {
    if ((s.deviceId.isIncluded() == Used::YES)
        && (s.deviceId.get() != deviceId)) {
      continue;
    }
    if (s.stateName.get().getValue() != type) {
      continue;
    }
    if (s.value.get() == state.toStdString()) {
      return true;
    }
  }
  return false;
}

AddStateCommand::AddStateCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, const QString &name, item::StateMachineType t, int actPos,
    QUndoCommand *parent) :
    EditStateCommand(c, devicePos, smPos, t,
        QObject::tr("Add DeviceState (to device: %1)").arg(devicePos), parent), name(
        name.toStdString())
{
  uint32_t actCount;
  vector<string> states;

  if (name.isEmpty()) {
    return;
  }

  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateMachineType::Discrete: {
        states = sm.relative.states;
        actCount = sm.discrete.states.size();
        break;
      }
      case item::StateMachineType::Relative: {
        states = sm.relative.states;
        actCount = states.size();
        break;
      }
      default:
        return;
    }
    if (find(states.begin(), states.end(), this->name) != states.end()) {
      //must be unique
      return;
    }

    if (actPos < 0) {
      //append
      actPos = actCount;
    }
    if (actPos > actCount) {
      return;
    }
    this->actPos = actPos;
    isValid = true;
  } catch (std::out_of_range&) {
  }
}

void AddStateCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineType::Discrete: {
      auto &actions = sm.discrete;
      actions.states.insert(actions.states.begin() + actPos, name);
      actions.enterStateAction.insert(actions.enterStateAction.begin() + actPos,
          item::DeviceAction());
      break;
    }
    case item::StateMachineType::Relative: {
      auto &states = sm.relative.states;
      states.insert(states.begin() + actPos, name);
      break;
    }
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void AddStateCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineType::Discrete: {
      auto &actions = sm.discrete;
      actions.states.erase(actions.states.begin() + actPos);
      actions.enterStateAction.erase(actions.enterStateAction.begin() + actPos);
      break;
    }
    case item::StateMachineType::Relative: {
      auto &states = sm.relative.states;
      states.erase(states.begin() + actPos);
      break;
    }
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

SetStateNameCommand::SetStateNameCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateMachineType t, uint32_t statePos,
    const QString &name, QUndoCommand *parent) :
    EditStateCommand(c, devicePos, smPos, t, QObject::tr("Edit state name"),
        parent), statePos(statePos), name(name.toStdString())
{
  if (name.isEmpty()) {
    return;
  }

  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateMachineType::Discrete:
        oldName = sm.discrete.states.at(statePos);
        break;
      case item::StateMachineType::Relative:
        oldName = sm.relative.states.at(statePos);
        break;
      default:
        return;
    }

    isValid = checkRemove(sm.smType.get().getValue(), qstr(oldName));
  } catch (std::out_of_range&) {
  }
}

void SetStateNameCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineType::Discrete:
      sm.discrete.states[statePos] = name;
      break;
    case item::StateMachineType::Relative:
      sm.relative.states[statePos] = name;
      break;
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void SetStateNameCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineType::Discrete:
      sm.discrete.states[statePos] = oldName;
      break;
    case item::StateMachineType::Relative:
      sm.relative.states[statePos] = oldName;
      break;
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

SetStateDeviceActionCommand::SetStateDeviceActionCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,
    uint32_t actPos, const item::DeviceAction &act, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Edit DeviceState action"), parent), c(c), devicePos(
        devicePos), smPos(smPos), actPos(actPos), t(t), act(act)
{
  auto *action = getActionFromSmRef(c, devicePos, smPos, t, actPos);
  if (action == nullptr) {
    return;
  }
  oldAct = *action;

  isValid = true;
}

void SetStateDeviceActionCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto *action = getActionFromSmRef(c, devicePos, smPos, t, actPos);
  *action = act;
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void SetStateDeviceActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto *action = getActionFromSmRef(c, devicePos, smPos, t, actPos);
  *action = oldAct;
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

bool SetStateDeviceActionCommand::valid() const
{
  return isValid;
}

AddActionCommand::AddActionCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateMachineAction t, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add DeviceAction (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos), t(t)
{
  optional<item::DeviceAction> *action = nullptr;

  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateMachineAction::Start:
        action = &sm.startAction;
        break;
      case item::StateMachineAction::Finish:
        action = &sm.finishAction;
        break;
      case item::StateMachineAction::Relative_Reset:
        action = &sm.relative.resetAction;
        break;
      case item::StateMachineAction::Relative_Next:
        action = &sm.relative.nextStateAction;
        break;
      case item::StateMachineAction::Relative_Prev:
        action = &sm.relative.prevStateAction;
        break;
      default:
        return;
    }
    if (action->has_value()) {
      //already exists
      return;
    }

    isValid = true;
  } catch (std::out_of_range&) {
  }
}

void AddActionCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineAction::Start: {
      auto &action = sm.startAction;
      action = item::DeviceAction();
      break;
    }
    case item::StateMachineAction::Finish: {
      auto &action = sm.finishAction;
      action = item::DeviceAction();
      break;
    }
    case item::StateMachineAction::Relative_Reset: {
      auto &action = sm.relative.resetAction;
      action = item::DeviceAction();
      break;
    }
    case item::StateMachineAction::Relative_Next: {
      auto &action = sm.relative.nextStateAction;
      action = item::DeviceAction();
      break;
    }
    case item::StateMachineAction::Relative_Prev: {
      auto &action = sm.relative.prevStateAction;
      action = item::DeviceAction();
      break;
    }
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void AddActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineAction::Start:
      sm.startAction = nullopt;
      break;
    case item::StateMachineAction::Finish:
      sm.finishAction = nullopt;
      break;
    case item::StateMachineAction::Relative_Reset:
      sm.relative.resetAction = nullopt;
      break;
    case item::StateMachineAction::Relative_Next:
      sm.relative.nextStateAction = nullopt;
      break;
    case item::StateMachineAction::Relative_Prev:
      sm.relative.prevStateAction = nullopt;
      break;
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

bool AddActionCommand::valid() const
{
  return isValid;
}

RemoveStateCommand::RemoveStateCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateMachineType t, uint32_t actPos,
    QUndoCommand *parent) :
    EditStateCommand(c, devicePos, smPos, t,
        QObject::tr("Remove DeviceState (Pos: %1)").arg(devicePos), parent), actPos(
        actPos)
{
  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateMachineType::Discrete:
        action = sm.discrete.enterStateAction.at(actPos);
        name = sm.discrete.states.at(actPos);
        break;
      case item::StateMachineType::Relative:
        name = sm.relative.states.at(actPos);
        break;
      default:
        return;
    }
    isValid = checkRemove(sm.smType.get().getValue(), qstr(name));
  } catch (std::out_of_range&) {
    return;
  }
}

void RemoveStateCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineType::Discrete: {
      auto &actions = sm.discrete;
      actions.states.erase(actions.states.begin() + actPos);
      actions.enterStateAction.erase(actions.enterStateAction.begin() + actPos);
      break;
    }
    case item::StateMachineType::Relative: {
      auto &states = sm.relative.states;
      states.erase(states.begin() + actPos);
      break;
    }
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void RemoveStateCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineType::Discrete: {
      auto &actions = sm.discrete;
      actions.states.insert(actions.states.begin() + actPos, name);
      actions.enterStateAction.insert(actions.enterStateAction.begin() + actPos,
          action);
      break;
    }
    case item::StateMachineType::Relative: {
      auto &states = sm.relative.states;
      states.insert(states.begin() + actPos, name);
      break;
    }
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

RemoveActionCommand::RemoveActionCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateMachineAction t, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove DeviceAction (Pos: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos), t(t)
{
  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateMachineAction::Start:
        action = sm.startAction.value();
        break;
      case item::StateMachineAction::Finish:
        action = sm.finishAction.value();
        break;
      case item::StateMachineAction::Relative_Reset:
        action = sm.relative.resetAction.value();
        break;
      case item::StateMachineAction::Relative_Next:
        action = sm.relative.nextStateAction.value();
        break;
      case item::StateMachineAction::Relative_Prev:
        action = sm.relative.prevStateAction.value();
        break;
      default:
        return;
    }
  } catch (std::bad_optional_access&) {
    return;
  }
  isValid = true;
}

void RemoveActionCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineAction::Start:
      sm.startAction = nullopt;
      break;
    case item::StateMachineAction::Finish:
      sm.finishAction = nullopt;
      break;
    case item::StateMachineAction::Relative_Reset:
      sm.relative.resetAction = nullopt;
      break;
    case item::StateMachineAction::Relative_Next:
      sm.relative.nextStateAction = nullopt;
      break;
    case item::StateMachineAction::Relative_Prev:
      sm.relative.prevStateAction = nullopt;
      break;
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void RemoveActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateMachineAction::Start: {
      auto &action = sm.startAction;
      action = this->action;
      break;
    }
    case item::StateMachineAction::Finish: {
      auto &action = sm.finishAction;
      action = this->action;
      break;
    }
    case item::StateMachineAction::Relative_Reset: {
      auto &action = sm.relative.resetAction;
      action = this->action;
      break;
    }
    case item::StateMachineAction::Relative_Next: {
      auto &action = sm.relative.nextStateAction;
      action = this->action;
      break;
    }
    case item::StateMachineAction::Relative_Prev: {
      auto &action = sm.relative.prevStateAction;
      action = this->action;
      break;
    }
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

bool RemoveActionCommand::valid() const
{
  return isValid;
}

}
}
