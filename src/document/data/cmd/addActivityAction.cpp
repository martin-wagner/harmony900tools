// SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>

#include "addActivityAction.h"

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

}
}
