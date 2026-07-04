// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeIrStream.h"

using namespace std;

namespace document
{
namespace data
{

RemoveIrStreamItemFromIrStreamCommand::RemoveIrStreamItemFromIrStreamCommand(uint32_t streamIndex,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove prot ir stream item from cmd"), parent)
{
  //todo implement this
  //todo also implement remove from states, remove from numpad
}

void RemoveIrStreamItemFromIrStreamCommand::redo()
{
}

void RemoveIrStreamItemFromIrStreamCommand::undo()
{
}

RemoveIrStreamtemCommand::RemoveIrStreamtemCommand(ConfigData &c, int pos,
    QUndoCommand *parent) :
        BaseCommand(QObject::tr("Remove IR stream item (Pos: %1)").arg(pos),
            parent), c(c), pos(pos)
{
  if ((pos < 0) || (c.getStreamLib().getStreamCount() >= pos)) {
    return;
  }
  stream = c.getStreamLib().accessStream(pos).accessStream();
  clock = c.getStreamLib().accessStream(pos).getClock();
  isValid = true;
}

void RemoveIrStreamtemCommand::redo()
{
  if (!isValid) {
    return;
  }

  // add config file commands for undo
  //todo
//    auto *cmd = new RemoveStatemachineFromActivityCommand(ids, a, this);
//
//    connect(cmd, &RemoveStatemachineFromActivityCommand::writeLog, this,
//        &RemoveStatemachineCommand::writeLog);
//    connect(cmd, &RemoveStatemachineFromActivityCommand::writeMsg, this,
//        &RemoveStatemachineCommand::writeMsg);
//    connect(cmd, &RemoveStatemachineFromActivityCommand::activityAboutToBeAdded, this,
//        &RemoveStatemachineCommand::activityAboutToBeAdded);
//    connect(cmd, &RemoveStatemachineFromActivityCommand::activityAdded, this,
//        &RemoveStatemachineCommand::activityAdded);
//    connect(cmd, &RemoveStatemachineFromActivityCommand::activityAboutToBeRemoved,
//        this, &RemoveStatemachineCommand::activityAboutToBeRemoved);
//    connect(cmd, &RemoveStatemachineFromActivityCommand::activityRemoved, this,
//        &RemoveStatemachineCommand::activityRemoved); todo
  c.getStreamLib().removeStream(pos);
  emit dirtyChanged(true);
}

void RemoveIrStreamtemCommand::undo()
{
  if (!isValid) {
    return;
  }

  // add config file commands for redo

  c.getStreamLib().insertStream(stream, clock, pos);
  emit dirtyChanged(true);
}

bool RemoveIrStreamtemCommand::valid() const
{
  return isValid;
}

}
}
