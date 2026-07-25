// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addPowerOnDevice.h"

using namespace std;

namespace document
{
namespace data
{

AddPowerOnDevice::AddPowerOnDevice(ConfigData &c, uint32_t activityPos,
    uint32_t id, int devicePos, bool overwrite, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set Power On (activity: %1)").arg(activityPos),
        parent), isValid(false), overwrite(overwrite), c(c), activityPos(
        activityPos), devicePos(devicePos), id(id)
{
  if (activityPos >= c.getActivities().size()) {
    return;
  }
  auto &ids = c.getActivities()[activityPos].getPowerOnDevices();
  //Device ids must be unique!
  for (const auto &existing : ids) {
    if (id == existing) {
      emit writeMsg(tr("Device %1 already exists").arg(id));
      return;
    }
  }

  uint32_t idCount = ids.size();

  if (devicePos < 0) {
    devicePos = static_cast<int>(idCount);
  }
  if (static_cast<uint32_t>(devicePos) > idCount) {
    return;
  }

  this->devicePos = devicePos;
  isValid = true;
}

void AddPowerOnDevice::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_POWER, devicePos);
  auto &roles = c.getActivities()[activityPos].getPowerOnDevices();
  roles.insert(roles.begin() + devicePos, id);
  emit itemAdded(Item::ACTIVITY_POWER, devicePos);
  emit dirtyChanged(true);
}

void AddPowerOnDevice::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY_POWER, devicePos);
  auto &roles = c.getActivities()[activityPos].getPowerOnDevices();
  roles.erase(roles.begin() + devicePos);
  emit itemRemoved(Item::ACTIVITY_POWER, devicePos);
  emit dirtyChanged(true);
}

bool AddPowerOnDevice::valid() const
{
  return isValid;
}

AddPowerOffDevice::AddPowerOffDevice(ConfigData &c, uint32_t activityPos,
    uint32_t id, int devicePos, bool overwrite, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set Power Off (activity: %1)").arg(activityPos),
        parent), isValid(false), overwrite(overwrite), c(c), activityPos(
        activityPos), devicePos(devicePos), id(id)
{
  if (activityPos >= c.getActivities().size()) {
    return;
  }
  auto &ids = c.getActivities()[activityPos].getPowerOffDevices();
  //Device ids must be unique!
  for (const auto &existing : ids) {
    if (id == existing) {
      emit writeMsg(tr("Device %1 already exists").arg(id));
      return;
    }
  }

  uint32_t idCount = ids.size();

  if (devicePos < 0) {
    devicePos = static_cast<int>(idCount);
  }
  if (static_cast<uint32_t>(devicePos) > idCount) {
    return;
  }

  this->devicePos = devicePos;
  isValid = true;
}

void AddPowerOffDevice::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_POWER, devicePos);
  auto &roles = c.getActivities()[activityPos].getPowerOffDevices();
  roles.insert(roles.begin() + devicePos, id);
  emit itemAdded(Item::ACTIVITY_POWER, devicePos);
  emit dirtyChanged(true);
}

void AddPowerOffDevice::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY_POWER, devicePos);
  auto &roles = c.getActivities()[activityPos].getPowerOffDevices();
  roles.erase(roles.begin() + devicePos);
  emit itemRemoved(Item::ACTIVITY_POWER, devicePos);
  emit dirtyChanged(true);
}

bool AddPowerOffDevice::valid() const
{
  return isValid;
}

}
}
