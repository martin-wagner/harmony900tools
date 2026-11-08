// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>

#include "button.h"
#include "device.h"

using namespace std;

namespace document
{
namespace data
{

AddDeviceCommand::AddDeviceCommand(ConfigData &c, int pos, QUndoCommand *parent) :
    BaseCommand("", parent), c(c), pos(pos)
{
  id = lib::UidGenerator::getInstance().generate();
  setText(QObject::tr("Add device (Id: %1, Pos: %2)").arg(id).arg(pos));

  if (c.getDevices().size() + 1 > ConfigData::DEVICE_LIMIT) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, tr("Add device"),
        tr("According to the manual, you can have a max of %1 devices. "
            "By adding another one you will exceed this number. You've "
            "been warned...").arg(ConfigData::DEVICE_LIMIT),
        QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
    if (reply == QMessageBox::Abort) {
      return;
    }
  }

  setPos();
  isValid = true;
}

AddDeviceCommand::AddDeviceCommand(ConfigData &c, uint32_t id, int pos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add device (Id: %1, Pos: %2)").arg(id).arg(pos),
        parent), c(c), id(id), pos(pos)
{
  if (c.getDevice(id) == nullptr) {
    isValid = true;
  }
  if (pos > c.getDevices().size()) {
    //append
    pos = -1;
  }
  setPos();
}

AddDeviceCommand::AddDeviceCommand(const item::Device &device, ConfigData &c,
    int pos, QUndoCommand *parent) :
    AddDeviceCommand(c, pos, parent)
{
  if (!isValid) {
    return;
  }

  this->device.emplace(device, id); //assing new id
}

void AddDeviceCommand::redo()
{
  if (!isValid) {
    return;
  }

  //todo newdevicefound property

  emit itemAboutToBeAdded(Item::DEVICE, pos);
  if (device.has_value()) {
    c.getDevices().insert(c.getDevices().begin() + pos, *device);
  } else {
    c.getDevices().insert(c.getDevices().begin() + pos, item::Device(id));
  }
  emit itemAdded(Item::DEVICE, pos);
  emit dirtyChanged(true);
}

void AddDeviceCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE, pos);
  c.getDevices().erase(c.getDevices().begin() + pos);
  emit itemRemoved(Item::DEVICE, pos);
  emit dirtyChanged(true);
}

uint32_t AddDeviceCommand::getUid() const
{
  return id;
}

bool AddDeviceCommand::valid() const
{
  return isValid;
}

void AddDeviceCommand::setPos()
{
  if (pos < 0) {
    pos = c.getDevices().size();
  }
}


RemoveDeviceCommand::RemoveDeviceCommand(ConfigData &c, uint32_t pos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove device (Pos: %1)").arg(pos), parent), c(c), pos(
        pos), device(0)
{
  if (pos >= c.getDevices().size()) {
    //beyond end
    return;
  }

  // copy device for undo
  device = c.getDevices()[pos];
  isValid = checkRemove();
}

void RemoveDeviceCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE, pos);
  QUndoCommand::redo();
  auto &devices = c.getDevices();
  devices.erase(devices.begin() + pos);
  emit itemRemoved(Item::DEVICE, pos);
  emit dirtyChanged(true);
}

void RemoveDeviceCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE, pos);
  auto &devices = c.getDevices();
  devices.insert(devices.begin() + pos, device);
  QUndoCommand::undo();
  emit itemAdded(Item::DEVICE, pos);
  emit dirtyChanged(true);
}

bool RemoveDeviceCommand::valid() const
{
  return isValid;
}

bool RemoveDeviceCommand::checkRemove()
{
  int i;
  int j;
  bool query = false;
  bool reject = false;
  QString activityStr;

  //fixme can we use signal/slot for this? the current implementation
  //requires all dependencies to be hard-coded.
  //we could also assign a uid to each IR command.

  //check the id in all activities
  auto &activities = c.getActivities();
  auto deviceId = device.getId();
  for (i = 0; i < activities.size(); i++) {
    for (const auto &action : activities[i].getEnterActions()) {
      reject |= checkActivityAction(action, deviceId);
    }
    for (const auto &action : activities[i].getLeaveActions()) {
      reject |= checkActivityAction(action, deviceId);
    }
    for (const auto &role : activities[i].getRoles()) {
      if (role.deviceId.get() == deviceId) {
        reject = true;
      }
    }
    auto &hardButtons = activities[i].getHardButtons();
    for (j = 0; j < hardButtons.size(); j++) {
      auto &button = hardButtons[j];
      if (button.device.get() == deviceId) {
        query = true;
      }
    }
    auto &softButtons = activities[i].getSoftButtons();
    for (j = 0; j < softButtons.size(); j++) {
      auto &button = softButtons[j];
      if (button.device.get() == deviceId) {
        query = true;
      }
    }
    if (reject || query) {
      activityStr = qstr(activities[i].label.get());
      break;
    }
  }
  if (reject) {
    QMessageBox::warning(nullptr, tr("Device is used"),
        tr("This device is used in at least one activity (%1). Remove it "
            "from there first.").arg(activityStr), QMessageBox::Ok);
    return false;
  }
  if (query) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, tr("Delete buttons"),
        tr("Buttons from this device are used in at least one activity (%1). "
            "Deleting the device will also delete all buttons.").arg(
            activityStr), QMessageBox::Ok | QMessageBox::Cancel);
    if (reply == QMessageBox::Ok) {
      return doRemove(deviceId);
    }
    return false;
  }
  return true;
}

bool RemoveDeviceCommand::checkActivityAction(const item::DeviceAction &d,
    uint32_t deviceId) const
{
  for (auto &s : d.sequence) {
    if (s.deviceId.get() == deviceId) {
      return true;
    }
  }
  return false;
}

bool RemoveDeviceCommand::doRemove(uint32_t deviceId)
{
  int i;
  int j;
  bool ret = true;

  //check the name in referencing activities
  auto &activities = c.getActivities();
  for (i = 0; i < activities.size(); i++) {
    auto &hardButtons = activities[i].getHardButtons();
    for (j = hardButtons.size() - 1; j >= 0; j--) {
      auto &button = hardButtons[j];
      if (button.device.get() == deviceId) {
        auto *cmd = new RemoveActivityButtonCommand(c, i,
            item::ButtonType::Hard, j, this);
        cmd->connectCommand(this);
        ret &= cmd->valid();
      }
    }
    auto &softButtons = activities[i].getSoftButtons();
    for (j = softButtons.size() - 1; j >= 0; j--) {
      auto &button = softButtons[j];
      if (button.device.get() == deviceId) {
        auto *cmd = new RemoveActivityButtonCommand(c, i,
            item::ButtonType::Soft, j, this);
        cmd->connectCommand(this);
        ret &= cmd->valid();
      }
    }
  }
  return ret;
}

}
}
