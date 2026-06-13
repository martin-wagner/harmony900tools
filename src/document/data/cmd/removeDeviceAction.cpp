// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeDeviceAction.h"

using namespace std;

namespace document
{
namespace data
{

RemoveDeviceActionFromActivityCommand::RemoveDeviceActionFromActivityCommand(
    const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent) :
        BaseCommand(QObject::tr("Remove DeviceAction from activity"), parent)
{
  // todo implement this!
}

void RemoveDeviceActionFromActivityCommand::redo()
{
}

void RemoveDeviceActionFromActivityCommand::undo()
{
}

RemoveDeviceActionCommand::RemoveDeviceActionCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, uint32_t actPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove DeviceAction (Pos: %1)").arg(devicePos), parent), c(
        c), devicePos(devicePos), smPos(smPos), actPos(actPos)
{
  uint32_t actCount;

  if (devicePos >= c.getDevices().size()) {
    return;
  }
  if (smPos >= c.getDevices()[devicePos].getStateMachines().size()) {
    return;
  }
  actCount = c.getDevices()[devicePos].getStateMachines()[smPos].getActions().size();
  if (actPos >= actCount) {
    return;
  }
  action = c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos];
  // add activities for undo
  for (auto &a : c.getActivities()) {
//    auto *cmd = new RemoveDeviceActionFromActivityCommand(ids, a, this);
//
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::writeLog, this,
//        &RemoveDeviceActionCommand::writeLog);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::writeMsg, this,
//        &RemoveDeviceActionCommand::writeMsg);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAboutToBeAdded, this,
//        &RemoveDeviceActionCommand::activityAboutToBeAdded);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAdded, this,
//        &RemoveDeviceActionCommand::activityAdded);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityAboutToBeRemoved,
//        this, &RemoveDeviceActionCommand::activityAboutToBeRemoved);
//    connect(cmd, &RemoveDeviceActionFromActivityCommand::activityRemoved, this,
//        &RemoveDeviceActionCommand::activityRemoved); todo
  }
  isValid = true;
}

void RemoveDeviceActionCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();


  auto &actions = c.getDevices()[devicePos].getStateMachines()[smPos].getActions();
  actions.erase(actions.begin() + actPos);
  emit dirtyChanged(true);
}

void RemoveDeviceActionCommand::undo()
{
  if (!isValid) {
    return;
  }
  auto &actions = c.getDevices()[devicePos].getStateMachines()[smPos].getActions();
  actions.insert(actions.begin() + actPos, action);
  QUndoCommand::undo();
  emit dirtyChanged(true);
}

bool RemoveDeviceActionCommand::valid() const
{
  return isValid;
}

}
}
