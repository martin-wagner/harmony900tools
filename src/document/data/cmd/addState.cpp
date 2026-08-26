// SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>

#include "addState.h"

using namespace std;

namespace document
{
namespace data
{

AddStateCommand::AddStateCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, const QString &name, item::StateMachineType t, int actPos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add DeviceState (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos), t(t), name(
        name.toStdString())
{
  uint32_t actCount;

  if (name.isEmpty()) {
    return;
  }

  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateMachineType::Discrete: {
        actCount = sm.discrete.states.size();
        break;
      }
      case item::StateMachineType::Relative: {
        auto &states = sm.relative.states;
        if (find(states.begin(), states.end(), this->name) != states.end()) {
          return;
        }
        actCount = states.size();
        break;
      }
      default:
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

bool AddStateCommand::valid() const
{
  return isValid;
}

EditStateNameCommand::EditStateNameCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, const QString &name, item::StateMachineType t,
    uint32_t statePos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Edit state name").arg(devicePos), parent), c(c), devicePos(
        devicePos), smPos(smPos), statePos(statePos), t(t), name(
        name.toStdString())
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

    isValid = true;
  } catch (std::out_of_range&) {
  }
}

void EditStateNameCommand::redo()
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

void EditStateNameCommand::undo()
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

bool EditStateNameCommand::valid() const
{
  return isValid;
}

AddActionCommand::AddActionCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateMachineAction t, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add DeviceAction (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos), t(t)
{
  switch (t) {
    case item::StateMachineAction::Start:
    case item::StateMachineAction::Finish:
    case item::StateMachineAction::Relative_Reset:
    case item::StateMachineAction::Relative_Next:
    case item::StateMachineAction::Relative_Prev:
      break;
    default:
      return;
  }
  if (devicePos >= c.getDevices().size()) {
    return;
  }
  if (smPos >= c.getDevices()[devicePos].getStateMachines().size()) {
    return;
  }

  isValid = true;
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

}
}
