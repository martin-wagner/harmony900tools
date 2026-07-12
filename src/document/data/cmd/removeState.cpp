// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setActionData.h"
#include "removeState.h"

using namespace std;

namespace document
{
namespace data
{

RemoveActionFromActivityCommand::RemoveActionFromActivityCommand(
    const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove DeviceAction from activity"), parent)
{
  // todo implement this!
}

void RemoveActionFromActivityCommand::redo()
{
}

void RemoveActionFromActivityCommand::undo()
{
}

RemoveStateCommand::RemoveStateCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateTransitionType t, uint32_t actPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove DeviceState (Pos: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos), t(t), actPos(actPos)
{
  uint32_t actCount;

  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateTransitionType::Discrete:
        action = sm.discrete.enterStateAction.at(actPos);
        name = sm.discrete.states.at(actPos);
        break;
      case item::StateTransitionType::Relative:
        name = sm.relative.states.at(actPos);
        break;
      default:
        return;
    }
    isValid = true;
  } catch (std::out_of_range&) {
    return;
  }

  // add activities for undo
  for (auto &a : c.getActivities()) {
//    auto *cmd = new RemoveDeviceActionFromActivityCommand(ids, a, this);
//
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::writeLog, this,
//        &RemoveDeviceActionCommand::writeLog);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::writeMsg, this,
//        &RemoveDeviceActionCommand::writeMsg);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAboutToBeAdded, this,
//        &RemoveDeviceActionCommand::activityAboutToBeAdded);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAdded, this,
//        &RemoveDeviceActionCommand::activityAdded);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAboutToBeRemoved,
//        this, &RemoveDeviceActionCommand::activityAboutToBeRemoved);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityRemoved, this,
//        &RemoveDeviceActionCommand::activityRemoved); todo
  }
  isValid = true;
}

void RemoveStateCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateTransitionType::Discrete: {
      auto &actions = sm.discrete;
      actions.states.erase(actions.states.begin() + actPos);
      actions.enterStateAction.erase(actions.enterStateAction.begin() + actPos);
      break;
    }
    case item::StateTransitionType::Relative: {
      auto &states = sm.relative.states;
      states.erase(states.begin() + actPos);
      break;
    }
    default:
      return;
  }
  emit dirtyChanged(true);
}

void RemoveStateCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateTransitionType::Discrete: {
      auto &actions = sm.discrete;
      actions.states.insert(actions.states.begin() + actPos, name);
      actions.enterStateAction.insert(actions.enterStateAction.begin() + actPos,
          action);
      break;
    }
    case item::StateTransitionType::Relative: {
      auto &states = sm.relative.states;
      states.insert(states.begin() + actPos, name);
      break;
    }
    default:
      return;
  }
  QUndoCommand::undo();
  emit dirtyChanged(true);
}

bool RemoveStateCommand::valid() const
{
  return isValid;
}

RemoveActionCommand::RemoveActionCommand(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateTransitionAction t, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove DeviceAction (Pos: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos), t(t)
{
  uint32_t actCount;

  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    switch (t) {
      case item::StateTransitionAction::Start:
        action = *sm.startAction;
        break;
      case item::StateTransitionAction::Finish:
        action = *sm.finishAction;
        break;
      case item::StateTransitionAction::Relative_Reset:
        action = *sm.relative.resetAction;
        break;
      case item::StateTransitionAction::Relative_Next:
        action = *sm.relative.nextStateAction;
        break;
      case item::StateTransitionAction::Relative_Prev:
        action = *sm.relative.prevStateAction;
        break;
      default:
        return;
    }
  } catch (std::out_of_range&) {
    return;
  }

  // add activities for undo
  for (auto &a : c.getActivities()) {
//    auto *cmd = new RemoveDeviceActionFromActivityCommand(ids, a, this);
//
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::writeLog, this,
//        &RemoveDeviceActionCommand::writeLog);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::writeMsg, this,
//        &RemoveDeviceActionCommand::writeMsg);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAboutToBeAdded, this,
//        &RemoveDeviceActionCommand::activityAboutToBeAdded);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAdded, this,
//        &RemoveDeviceActionCommand::activityAdded);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAboutToBeRemoved,
//        this, &RemoveDeviceActionCommand::activityAboutToBeRemoved);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityRemoved, this,
//        &RemoveDeviceActionCommand::activityRemoved); todo
  }
  isValid = true;
}

void RemoveActionCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateTransitionAction::Start:
      sm.startAction = nullopt;
      break;
    case item::StateTransitionAction::Finish:
      sm.finishAction = nullopt;
      break;
    case item::StateTransitionAction::Relative_Reset:
      sm.relative.resetAction = nullopt;
      break;
    case item::StateTransitionAction::Relative_Next:
      sm.relative.nextStateAction = nullopt;
      break;
    case item::StateTransitionAction::Relative_Prev:
      sm.relative.prevStateAction = nullopt;
      break;
    default:
      return;
  }
  emit dirtyChanged(true);
}

void RemoveActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  switch (t) {
    case item::StateTransitionAction::Start: {
      auto &action = sm.startAction;
      action = this->action;
      break;
    }
    case item::StateTransitionAction::Finish: {
      auto &action = sm.finishAction;
      action = this->action;
      break;
    }
    case item::StateTransitionAction::Relative_Reset: {
      auto &action = sm.relative.resetAction;
      action = this->action;
      break;
    }
    case item::StateTransitionAction::Relative_Next: {
      auto &action = sm.relative.nextStateAction;
      action = this->action;
      break;
    }
    case item::StateTransitionAction::Relative_Prev: {
      auto &action = sm.relative.prevStateAction;
      action = this->action;
      break;
    }
    default:
      return;
  }
  QUndoCommand::undo();
  emit dirtyChanged(true);
}

bool RemoveActionCommand::valid() const
{
  return isValid;
}

}
}
