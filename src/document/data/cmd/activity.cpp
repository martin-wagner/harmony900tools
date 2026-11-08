// SPDX-License-Identifier: LGPL-2.1-or-later

#include "activity.h"

using namespace std;

namespace document
{
namespace data
{

AddActivityCommand::AddActivityCommand(ConfigData &c, int pos, QUndoCommand *parent) :
    BaseCommand("", parent), c(c), pos(pos)
{
  id = lib::UidGenerator::getInstance().generate();
  setText(QObject::tr("Add activity (Id: %1, Pos: %2)").arg(id).arg(pos));
  setPos();
  isValid = true;
}

AddActivityCommand::AddActivityCommand(ConfigData &c, uint32_t id, int pos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add activity (Id: %1, Pos: %2)").arg(id).arg(pos),
        parent), c(c), id(id), pos(pos)
{
  if (c.getActivity(id) == nullptr) {
    isValid = true;
  }
  if (pos > c.getActivities().size()) {
    //append
    pos = -1;
  }
  setPos();
}

void AddActivityCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY, pos);
  c.getActivities().insert(c.getActivities().begin() + pos, item::Activity(id));
  emit itemAdded(Item::ACTIVITY, pos);
  emit dirtyChanged(true);
}

void AddActivityCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY, pos);
  c.getActivities().erase(c.getActivities().begin() + pos);
  emit itemRemoved(Item::ACTIVITY, pos);
  emit dirtyChanged(true);
}

uint32_t AddActivityCommand::getUid() const
{
  return id;
}

bool AddActivityCommand::valid() const
{
  return isValid;
}

void AddActivityCommand::setPos()
{
  if (pos < 0) {
    pos = c.getActivities().size();
  }
}


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
