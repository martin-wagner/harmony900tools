// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeIrProto.h"

using namespace std;

namespace document
{
namespace data
{

RemoveIrProtoLibItemFromIrProtoCommand::RemoveIrProtoLibItemFromIrProtoCommand(uint32_t protocolIndex,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove prot ir lib item from cmd"), parent)
{
  //todo implement this
  //todo also implement remove from states, remove from numpad
}

void RemoveIrProtoLibItemFromIrProtoCommand::redo()
{
}

void RemoveIrProtoLibItemFromIrProtoCommand::undo()
{
}

RemoveIrProtoLibItemCommand::RemoveIrProtoLibItemCommand(ConfigData &c, int pos,
    QUndoCommand *parent) :
        BaseCommand(QObject::tr("Remove IR Proto Lib item (Pos: %1)").arg(pos),
            parent), c(c), pos(pos)
{
  if ((pos < 0) || (c.getProtocolLib().getProtocolCount() >= pos)) {
    return;
  }
  prot = c.getProtocolLib().accessProtocol(pos);
  isValid = true;
}

void RemoveIrProtoLibItemCommand::redo()
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
  c.getProtocolLib().removeProtocol(pos);
  emit dirtyChanged(true);
}

void RemoveIrProtoLibItemCommand::undo()
{
  if (!isValid) {
    return;
  }

  // add config file commands for redo

  c.getProtocolLib().insertProtocol(prot, pos);
  emit dirtyChanged(true);
}

bool RemoveIrProtoLibItemCommand::valid() const
{
  return isValid;
}

}
}
