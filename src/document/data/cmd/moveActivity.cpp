// SPDX-License-Identifier: LGPL-2.1-or-later

#include "moveActivity.h"

using namespace std;

namespace document
{
namespace data
{

MoveActivityCommand::MoveActivityCommand(ConfigData &c, uint32_t currentPos,
    uint32_t newPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Move activity (From: %1, To: %2)").arg(currentPos).arg(
            newPos), parent), c(c), oldPos(currentPos), newPos(newPos)
{
  if ((currentPos > c.getActivities().size())
      || (newPos > c.getActivities().size())) {
    return;
  }
  isValid = true;
}

void MoveActivityCommand::redo()
{
  work(oldPos, newPos);
}

void MoveActivityCommand::undo()
{
  work(newPos, oldPos);
}

void MoveActivityCommand::work(uint32_t posA, uint32_t posB)
{
  if (!isValid) {
    return;
  }
  if (posA == posB) {
    return;
  }

  auto activity = c.getActivities()[posA];
  remove(posA);
  add(activity, posB);
  emit dirtyChanged(true);
}

void MoveActivityCommand::remove(uint32_t pos)
{
  emit itemAboutToBeRemoved(Item::ACTIVITY, pos);
  auto &activities = c.getActivities();
  activities.erase(activities.begin() + pos);
  emit itemRemoved(Item::ACTIVITY, pos);
}

void MoveActivityCommand::add(item::Activity &act, uint32_t pos)
{
  emit itemAboutToBeAdded(Item::ACTIVITY, pos);
  auto &activities = c.getActivities();
  activities.insert(activities.begin() + pos, act);
  emit itemAdded(Item::ACTIVITY, pos);
}

bool MoveActivityCommand::valid() const
{
  return isValid;
}

}
}
