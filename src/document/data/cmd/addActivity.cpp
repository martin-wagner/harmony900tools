// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addActivity.h"

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
  emit activityAboutToBeAdded(pos);
  c.getActivities().insert(c.getActivities().begin() + pos, item::Activity(id));
  emit activityAdded(pos);
  emit dirtyChanged(true);
}

void AddActivityCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit activityAboutToBeRemoved(pos);
  c.getActivities().erase(c.getActivities().begin() + pos);
  emit activityRemoved(pos);
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

}
}
