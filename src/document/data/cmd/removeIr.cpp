// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeIr.h"

using namespace std;

namespace document
{
namespace data
{

RemoveIrFromButtonCommand::RemoveIrFromButtonCommand(const std::string &name,
    uint32_t devicePos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Cmd from buttons"), parent)
{
  //todo implement this
  //todo also implement remove from states, remove from numpad
}

void RemoveIrFromButtonCommand::redo()
{
}

void RemoveIrFromButtonCommand::undo()
{
}

RemoveIrProtoCommand::RemoveIrProtoCommand(ConfigData &c, uint32_t devicePos,
    uint32_t cmdPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Proto Cmd (Pos: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), cmdPos(cmdPos)
{
  uint32_t cmdCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }

  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  cmdCount = cmds.getProtoCommands().size();
  if (cmdPos >= cmdCount) {
    return;
  }
  proto = cmds.getProtoCommands()[cmdPos];
  // add button for undo
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
  isValid = true;
}

void RemoveIrProtoCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_IR_DATA, cmdPos);
  QUndoCommand::redo();
  auto &pcmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
  pcmds.erase(pcmds.begin() + cmdPos);
  emit itemRemoved(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

void RemoveIrProtoCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_IR_DATA, cmdPos);
  auto &pcmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
  pcmds.insert(pcmds.begin() + cmdPos, proto);
  QUndoCommand::undo();
  emit itemAdded(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

bool RemoveIrProtoCommand::valid() const
{
  return isValid;
}

RemoveIrRawCommand::RemoveIrRawCommand(ConfigData &c, uint32_t devicePos,
    uint32_t cmdPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Raw Cmd (Pos: %1)").arg(devicePos), parent), c(
        c), devicePos(devicePos), cmdPos(cmdPos)
{
  uint32_t cmdCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }

  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  cmdCount = cmds.getRawCommands().size();
  if (cmdPos >= cmdCount) {
    return;
  }
  raw = cmds.getRawCommands()[cmdPos];
  // add button for undo
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
  isValid = true;
}

void RemoveIrRawCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_IR_DATA, cmdPos);
  QUndoCommand::redo();
  auto &rcmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
  rcmds.erase(rcmds.begin() + cmdPos);
  emit itemRemoved(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

void RemoveIrRawCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_IR_DATA, cmdPos);
  auto &rcmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
  rcmds.insert(rcmds.begin() + cmdPos, raw);
  QUndoCommand::undo();
  emit itemAdded(Item::DEVICE_IR_DATA, cmdPos);
  emit dirtyChanged(true);
}

bool RemoveIrRawCommand::valid() const
{
  return isValid;
}

}
}
