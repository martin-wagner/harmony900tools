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

  emit itemAboutToBeRemoved(Item::ACTIVITY, pos);
  auto &activities = c.getActivities();
  activities.erase(activities.begin() + pos);
  emit itemRemoved(Item::ACTIVITY, pos);
  emit dirtyChanged(true);
}

void RemoveActivityCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY, pos);
  auto &activities = c.getActivities();
  activities.insert(activities.begin() + pos, activity);
  emit itemAdded(Item::ACTIVITY, pos);
  emit dirtyChanged(true);
}


bool RemoveActivityCommand::valid() const
{
  return isValid;
}

}
}
