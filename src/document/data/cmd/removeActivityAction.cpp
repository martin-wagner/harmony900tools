// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setActionData.h"
#include "removeActivityAction.h"

using namespace std;

namespace document
{
namespace data
{

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
      return;
  }
  emit dirtyChanged(true);
}

void RemoveActivityActionCommand::undo()
{
  if (!isValid) {
    return;
  }

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
      return;
  }
  emit dirtyChanged(true);
}

bool RemoveActivityActionCommand::valid() const
{
  return isValid;
}


}
}
