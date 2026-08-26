// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>

#include "removeButton.h"
#include "removeDevice.h"

using namespace std;

namespace document
{
namespace data
{

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
