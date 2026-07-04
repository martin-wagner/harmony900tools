// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeActivity.h"

using namespace std;

namespace document
{
namespace data
{

RemoveActivityCommand::RemoveActivityCommand(ConfigData &c, uint32_t pos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove activity (Pos: %1)").arg(pos), parent), c(c), pos(
        pos), activity(0)
{
  if (pos >= c.getActivities().size()) {
    //beyond end
    return;
  }

  // copy activity for undo
  activity = c.getActivities()[pos];
  isValid = true;
}

void RemoveActivityCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit activityAboutToBeRemoved(pos);
  QUndoCommand::redo();

  auto &activities = c.getActivities();
  activities.erase(activities.begin() + pos);
  emit activityRemoved(pos);
  emit dirtyChanged(true);
}

void RemoveActivityCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &activities = c.getActivities();
  activities.insert(activities.begin() + pos, activity);
  QUndoCommand::undo();
  emit activityAdded(pos);
  emit dirtyChanged(true);
}


bool RemoveActivityCommand::valid() const
{
  return isValid;
}

}
}
