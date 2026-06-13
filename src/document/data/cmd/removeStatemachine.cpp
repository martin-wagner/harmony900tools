// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeStatemachine.h"

using namespace std;

namespace document
{
namespace data
{

RemoveStatemachineFromActivityCommand::RemoveStatemachineFromActivityCommand(
    const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent) :
        BaseCommand(QObject::tr("Remove Statemachine from activity"), parent)
{
  // todo implement this!
}

void RemoveStatemachineFromActivityCommand::redo()
{
}

void RemoveStatemachineFromActivityCommand::undo()
{
}

RemoveStatemachineCommand::RemoveStatemachineCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Statemachine (Pos: %1)").arg(devicePos), parent), c(
        c), devicePos(devicePos), smPos(smPos)
{
  uint32_t smCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }
  smCount = c.getDevices()[devicePos].getStateMachines().size();
  if (smPos >= smCount) {
    return;
  }
  sm = c.getDevices()[devicePos].getStateMachines()[smPos];
  // add activities for undo
  for (auto &a : c.getActivities()) {
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
  }
  isValid = true;
}

void RemoveStatemachineCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();


  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.erase(sms.begin() + smPos);
  emit dirtyChanged(true);
}

void RemoveStatemachineCommand::undo()
{
  if (!isValid) {
    return;
  }
  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.insert(sms.begin() + smPos, sm);
  QUndoCommand::undo();
  emit dirtyChanged(true);
}

bool RemoveStatemachineCommand::valid() const
{
  return isValid;
}

}
}
