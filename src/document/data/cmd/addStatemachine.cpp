// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addStatemachine.h"

using namespace std;

namespace document
{
namespace data
{

AddStatemachineCommand::AddStatemachineCommand(ConfigData &c,
    uint32_t devicePos, int smPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Statemachine (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos)
{
  uint32_t smCount;

  if (devicePos >= c.getDevices().size()) {
    return;
  }
  smCount = c.getDevices()[devicePos].getStateMachines().size();
  if (smPos < 0) {
    //append
    smPos = smCount;
  }
  if (smPos > smCount) {
    return;
  }
  this->smPos = smPos;
  isValid = true;
}

void AddStatemachineCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_STATEMACHINE, smPos);
  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.insert(sms.begin() + smPos, item::StateMachine());
  emit itemAdded(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void AddStatemachineCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_STATEMACHINE, smPos);
  auto &sms = c.getDevices()[devicePos].getStateMachines();
  sms.erase(sms.begin() + smPos);
  emit itemRemoved(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

bool AddStatemachineCommand::valid() const
{
  return isValid;
}

SetStatemachineCommand::SetStatemachineCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, const item::StateMachine &sm,
    QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Update Statemachine (in device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos), sm(sm)
{
  try {
    oldSm = c.getDevices().at(devicePos).getStateMachines().at(smPos);
    isValid = true;
  } catch (std::out_of_range&) {
  }
}

void SetStatemachineCommand::redo()
{
  if (!isValid) {
    return;
  }

  c.getDevices()[devicePos].getStateMachines()[smPos] = sm;

  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

void SetStatemachineCommand::undo()
{
  if (!isValid) {
    return;
  }

  c.getDevices()[devicePos].getStateMachines()[smPos] = oldSm;

  emit itemChanged(Item::DEVICE_STATEMACHINE, smPos);
  emit dirtyChanged(true);
}

bool SetStatemachineCommand::valid() const
{
  return isValid;
}

}
}
