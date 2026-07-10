// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removePowerOnDevice.h"

using namespace std;

namespace document
{
namespace data
{

RemovePowerOnDevice* RemovePowerOnDevice::fromId(ConfigData &c,
    uint32_t activityPos, uint32_t deviceId, QUndoCommand *parent)
{
  auto &devices = c.getActivities()[activityPos].getPowerOnDevices();
  for (uint32_t i = 0; i < devices.size(); i++) {
    if (devices[i] == deviceId) {
      new RemovePowerOnDevice(c, activityPos, i, parent);
    }
  }
  return nullptr;
}

RemovePowerOnDevice::RemovePowerOnDevice(ConfigData &c, uint32_t activityPos,
    uint32_t devicePos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove power on device (from activity: %1)").arg(
            activityPos), parent), c(c), activityPos(activityPos), devicePos(
        devicePos)
{
  uint32_t roleCount;

  if (activityPos >= c.getActivities().size()) {
    //beyond end
    return;
  }

  auto &devices = c.getActivities()[activityPos].getPowerOnDevices();
  roleCount = devices.size();
  if (devicePos >= roleCount) {
    return;
  }
  id = devices[devicePos];
  isValid = true;
}

void RemovePowerOnDevice::redo()
{
  if (!isValid) {
    return;
  }

  auto &devices = c.getActivities()[activityPos].getPowerOnDevices();
  devices.erase(devices.begin() + devicePos);
  emit dirtyChanged(true);
}

void RemovePowerOnDevice::undo()
{
  if (!isValid) {
    return;
  }

  auto &devices = c.getActivities()[activityPos].getPowerOnDevices();
  devices.insert(devices.begin() + devicePos, id);

  emit dirtyChanged(true);
}

bool RemovePowerOnDevice::valid() const
{
  return isValid;
}

RemovePowerOffDevice* RemovePowerOffDevice::fromId(ConfigData &c,
    uint32_t activityPos, uint32_t deviceId, QUndoCommand *parent)
{
  auto &devices = c.getActivities()[activityPos].getPowerOffDevices();
  for (uint32_t i = 0; i < devices.size(); i++) {
    if (devices[i] == deviceId) {
      new RemovePowerOffDevice(c, activityPos, i, parent);
    }
  }
  return nullptr;
}

RemovePowerOffDevice::RemovePowerOffDevice(ConfigData &c, uint32_t activityPos,
    uint32_t devicePos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove power off device (from activity: %1)").arg(
            activityPos), parent), c(c), activityPos(activityPos), devicePos(
        devicePos)
{
  uint32_t roleCount;

  if (activityPos >= c.getActivities().size()) {
    //beyond end
    return;
  }

  auto &devices = c.getActivities()[activityPos].getPowerOffDevices();
  roleCount = devices.size();
  if (devicePos >= roleCount) {
    return;
  }
  id = devices[devicePos];
  isValid = true;
}

void RemovePowerOffDevice::redo()
{
  if (!isValid) {
    return;
  }

  auto &devices = c.getActivities()[activityPos].getPowerOffDevices();
  devices.erase(devices.begin() + devicePos);
  emit dirtyChanged(true);
}

void RemovePowerOffDevice::undo()
{
  if (!isValid) {
    return;
  }

  auto &devices = c.getActivities()[activityPos].getPowerOffDevices();
  devices.insert(devices.begin() + devicePos, id);

  emit dirtyChanged(true);
}

bool RemovePowerOffDevice::valid() const
{
  return isValid;
}

}
}
