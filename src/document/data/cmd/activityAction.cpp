// SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>

#include "getActionData.h"
#include "activityAction.h"

using namespace std;

namespace document
{
namespace data
{

AddActivityActionCommand::AddActivityActionCommand(ConfigData &c,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Action (to activity: %1)").arg(activityPos),
        parent), c(c), activityPos(activityPos), t(t)
{
  uint32_t actCount;

  try {
    auto &activity = c.getActivities().at(activityPos);
    switch (t) {
      case item::ActivityAction::Enter:
        actCount = activity.getEnterActions().size();
        break;
      case item::ActivityAction::Leave:
        actCount = activity.getLeaveActions().size();
        break;
      default:
        return;
    }

    if (actionPos < 0) {
      //append
      actionPos = actCount;
    }
    if (actionPos > actCount) {
      return;
    }
    this->actionPos = actionPos;
    isValid = true;
  } catch (std::out_of_range&) {
  }
}

void AddActivityActionCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_ACTION, actionPos);
  auto &activity = c.getActivities()[activityPos];
  switch (t) {
    case item::ActivityAction::Enter: {
      auto &actions = activity.getEnterActions();
      actions.insert(actions.begin() + actionPos, item::DeviceAction());
      break;
    }
    case item::ActivityAction::Leave: {
      auto &actions = activity.getLeaveActions();
      actions.insert(actions.begin() + actionPos, item::DeviceAction());
      break;
    }
    default:
      break;
  }
  emit itemAdded(Item::ACTIVITY_ACTION, actionPos);
  emit dirtyChanged(true);
}

void AddActivityActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY_ACTION, actionPos);
  auto &activity = c.getActivities()[activityPos];
  switch (t) {
    case item::ActivityAction::Enter: {
      auto &actions = activity.getEnterActions();
      actions.erase(actions.begin() + actionPos);
      break;
    }
    case item::ActivityAction::Leave: {
      auto &actions = activity.getLeaveActions();
      actions.erase(actions.begin() + actionPos);
      break;
    }
    default:
      break;
  }
  emit itemRemoved(Item::ACTIVITY_ACTION, actionPos);
  emit dirtyChanged(true);
}

bool AddActivityActionCommand::valid() const
{
  return isValid;
}


RemoveActivityActionCommand::RemoveActivityActionCommand(ConfigData &c, uint32_t activityPos,
    item::ActivityAction t, uint32_t actionPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Activity Action (Pos: %1)").arg(activityPos),
        parent), c(c), activityPos(activityPos),  t(t), actionPos(actionPos)
{
  auto *action = getActionFromActivity(c, activityPos, t, actionPos);
  if (action == nullptr) {
    return;
  }
  this->action = *action;
  isValid = true;
}

void RemoveActivityActionCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY_ACTION, actionPos);
  auto &activity = c.getActivities()[activityPos];
  switch (t) {
    case item::ActivityAction::Enter: {
      auto &actions = activity.getEnterActions();
      actions.erase(actions.begin() + actionPos);
      break;
    }
    case item::ActivityAction::Leave: {
      auto &actions = activity.getLeaveActions();
      actions.erase(actions.begin() + actionPos);
      break;
    }
    default:
      break;
  }
  emit itemRemoved(Item::ACTIVITY_ACTION, actionPos);
  emit dirtyChanged(true);
}

void RemoveActivityActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_ACTION, actionPos);
  auto &activity = c.getActivities()[activityPos];
  switch (t) {
    case item::ActivityAction::Enter: {
      auto &actions = activity.getEnterActions();
      actions.insert(actions.begin() + actionPos, action);
      break;
    }
    case item::ActivityAction::Leave: {
      auto &actions = activity.getLeaveActions();
      actions.insert(actions.begin() + actionPos, action);
      break;
    }
    default:
      break;
  }
  emit itemAdded(Item::ACTIVITY_ACTION, actionPos);
  emit dirtyChanged(true);
}

bool RemoveActivityActionCommand::valid() const
{
  return isValid;
}


SetActivityActionCommand::SetActivityActionCommand(ConfigData &c,
    uint32_t activityPos, item::ActivityAction t, int actionPos,
    const item::DeviceAction &act, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Edit Activity action"), parent), c(c), activityPos(
        activityPos), t(t), actionPos(actionPos), act(act)
{
  auto *action = getActionFromActivity(c, activityPos, t, actionPos);
  if (action == nullptr) {
    return;
  }
  oldAct = *action;

  isValid = true;
}
void SetActivityActionCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto *action = getActionFromActivity(c, activityPos, t, actionPos);
  *action = act;
  emit itemChanged(Item::ACTIVITY_ACTION, activityPos);
  emit dirtyChanged(true);
}
void SetActivityActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto *action = getActionFromActivity(c, activityPos, t, actionPos);
  *action = oldAct;
  emit itemChanged(Item::ACTIVITY_ACTION, activityPos);
  emit dirtyChanged(true);
}
bool SetActivityActionCommand::valid() const
{
  return isValid;
}


}
}
