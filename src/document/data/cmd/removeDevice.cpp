// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeDevice.h"

using namespace std;

namespace document
{
namespace data
{

RemoveDeviceFromActivityCommand::RemoveDeviceFromActivityCommand(
    set<uint32_t> ids, item::Activity &a, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove device from activity"), parent)
{
  // todo implement this!
}

void RemoveDeviceFromActivityCommand::redo()
{
}

void RemoveDeviceFromActivityCommand::undo()
{
}

RemoveDeviceCommand::RemoveDeviceCommand(ConfigData &c, int pos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove device (Pos: %1)").arg(pos), parent), c(c), pos(
        pos), device(0)
{
  if (pos > c.getDevices().size()) {
    //beyond end
    return;
  }

  // copy device for undo
  device = c.getDevices()[pos];

  // add activities for undo
  auto ids = device.getAllIds();
  for (auto &a : c.getActivities()) {
    auto *cmd = new RemoveDeviceFromActivityCommand(ids, a, this);

    connect(cmd, &RemoveDeviceFromActivityCommand::writeLog, this,
        &RemoveDeviceCommand::writeLog);
    connect(cmd, &RemoveDeviceFromActivityCommand::writeMsg, this,
        &RemoveDeviceCommand::writeMsg);
    connect(cmd, &RemoveDeviceFromActivityCommand::activityAboutToBeAdded, this,
        &RemoveDeviceCommand::activityAboutToBeAdded);
    connect(cmd, &RemoveDeviceFromActivityCommand::activityAdded, this,
        &RemoveDeviceCommand::activityAdded);
    connect(cmd, &RemoveDeviceFromActivityCommand::activityAboutToBeRemoved,
        this, &RemoveDeviceCommand::activityAboutToBeRemoved);
    connect(cmd, &RemoveDeviceFromActivityCommand::activityRemoved, this,
        &RemoveDeviceCommand::activityRemoved);
  }

  isValid = true;
}

void RemoveDeviceCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit deviceAboutToBeRemoved(pos);
  QUndoCommand::redo();

  auto &devices = c.getDevices();
  device = devices[pos]; //copy
  devices.erase(devices.begin() + pos);
  emit deviceRemoved(pos);
  emit dirtyChanged(true);
}

void RemoveDeviceCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &devices = c.getDevices();
  devices.insert(devices.begin() + pos, device);
  QUndoCommand::undo();
  emit deviceAdded(pos);
  emit dirtyChanged(true);
}

bool RemoveDeviceCommand::valid() const
{
  return isValid;
}

}
}
