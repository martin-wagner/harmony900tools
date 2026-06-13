// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addDeviceAction.h"

using namespace std;

namespace document
{
namespace data
{

AddDeviceActionCommand::AddDeviceActionCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, int actPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Button (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), smPos(smPos)
{
  uint32_t actCount;

  if (devicePos >= c.getDevices().size()) {
    return;
  }
  if (smPos >= c.getDevices()[devicePos].getStateMachines().size()) {
    return;
  }
  actCount = c.getDevices()[devicePos].getStateMachines()[smPos].getActions().size();
  if (actPos < 0) {
    //append
    actPos = actCount;
  }
  if (actPos > actCount) {
    return;
  }
  this->actPos = actPos;
  isValid = true;
}

void AddDeviceActionCommand::redo()
{
  if (!isValid) {
    return;
  }
  auto &actions = c.getDevices()[devicePos].getStateMachines()[smPos].getActions();
  actions.insert(actions.begin() + actPos, item::DeviceAction());
  emit dirtyChanged(true);
}

void AddDeviceActionCommand::undo()
{
  if (!isValid) {
    return;
  }
  auto &actions = c.getDevices()[devicePos].getStateMachines()[smPos].getActions();
  actions.erase(actions.begin() + actPos);
  emit dirtyChanged(true);
}

bool AddDeviceActionCommand::valid() const
{
  return isValid;
}

}
}
