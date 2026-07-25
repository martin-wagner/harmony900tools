// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addStatemachine.h"

using namespace std;

namespace document
{
namespace data
{

AddStatemachineCommand::AddStatemachineCommand(ConfigData &c,
    uint32_t devicePos, int smPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Button (to device: %1)").arg(devicePos),
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

}
}
