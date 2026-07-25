// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addButton.h"

using namespace std;

namespace document
{
namespace data
{

AddDeviceButtonCommand::AddDeviceButtonCommand(ConfigData &c, item::ButtonType t,
    uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Button (to device: %1)").arg(devicePos),
        parent), c(c), type(t), devicePos(devicePos)
{
  uint32_t buttonCount;

  if (devicePos >= c.getDevices().size()) {
    return;
  }
  buttonCount = c.getDevices()[devicePos].getButtons().size();
  if (buttonPos < 0) {
    //append
    buttonPos = buttonCount;
  }
  if (buttonPos > buttonCount) {
    return;
  }
  this->buttonPos = buttonPos;
  isValid = true;
}

void AddDeviceButtonCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_BUTTON, buttonPos);
  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.insert(buttons.begin() + buttonPos, item::Button(type));
  emit itemAdded(Item::DEVICE_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

void AddDeviceButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_BUTTON, buttonPos);
  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.erase(buttons.begin() + buttonPos);
  emit itemRemoved(Item::DEVICE_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

bool AddDeviceButtonCommand::valid() const
{
  return isValid;
}

AddActivityButtonCommand::AddActivityButtonCommand(ConfigData &c, item::ButtonType t,
    uint32_t activityPos, int buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Button (to activity: %1)").arg(activityPos),
        parent), c(c), type(t), activityPos(activityPos)
{
  uint32_t buttonCount;

  if (activityPos >= c.getActivities().size()) {
    return;
  }
  buttonCount = c.getActivities()[activityPos].getButtons().size();
  if (buttonPos < 0) {
    //append
    buttonPos = buttonCount;
  }
  if (buttonPos > buttonCount) {
    return;
  }
  this->buttonPos = buttonPos;
  isValid = true;
}

void AddActivityButtonCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_BUTTON, buttonPos);
  auto &buttons = c.getActivities()[activityPos].getButtons();
  buttons.insert(buttons.begin() + buttonPos, item::Button(type));
  emit itemAdded(Item::ACTIVITY_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

void AddActivityButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY_BUTTON, buttonPos);
  auto &buttons = c.getActivities()[activityPos].getButtons();
  buttons.erase(buttons.begin() + buttonPos);
  emit itemRemoved(Item::ACTIVITY_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

bool AddActivityButtonCommand::valid() const
{
  return isValid;
}

}
}
