// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addButton.h"

using namespace std;

namespace document
{
namespace data
{

AddDeviceButtonCommand::AddDeviceButtonCommand(ConfigData &c,
    uint32_t devicePos, item::ButtonType t, int buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Button (to device: %1)").arg(devicePos),
        parent), c(c), type(t), devicePos(devicePos)
{
  uint32_t buttonCount;

  if (devicePos >= c.getDevices().size()) {
    return;
  }
  switch (type) {
    case item::ButtonType::Hard:
      buttonCount = c.getDevices()[devicePos].getHardButtons().size();
      break;
    case item::ButtonType::Soft:
      buttonCount = c.getDevices()[devicePos].getSoftButtons().size();
      break;
    default:
      return;
  }
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

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeAdded(Item::DEVICE_HARD_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getHardButtons();
      buttons.insert(buttons.begin() + buttonPos, item::Button());
      emit itemAdded(Item::DEVICE_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeAdded(Item::DEVICE_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getSoftButtons();
      buttons.insert(buttons.begin() + buttonPos, item::Button());
      emit itemAdded(Item::DEVICE_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

void AddDeviceButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeAdded(Item::DEVICE_HARD_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getHardButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemAdded(Item::DEVICE_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeAdded(Item::DEVICE_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getSoftButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemAdded(Item::DEVICE_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

bool AddDeviceButtonCommand::valid() const
{
  return isValid;
}

AddActivityButtonCommand::AddActivityButtonCommand(ConfigData &c,
    uint32_t activityPos, item::ButtonType t, int buttonPos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Button (to activity: %1)").arg(activityPos),
        parent), c(c), type(t), activityPos(activityPos)
{
  uint32_t buttonCount;

  if (activityPos >= c.getActivities().size()) {
    return;
  }
  switch (type) {
    case item::ButtonType::Hard:
      buttonCount = c.getActivities()[activityPos].getHardButtons().size();
      break;
    case item::ButtonType::Soft:
      buttonCount = c.getActivities()[activityPos].getSoftButtons().size();
      break;
    default:
      return;
  }
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

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeAdded(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getHardButtons();
      buttons.insert(buttons.begin() + buttonPos, item::Button());
      emit itemAdded(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeAdded(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getSoftButtons();
      buttons.insert(buttons.begin() + buttonPos, item::Button());
      emit itemAdded(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

void AddActivityButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeAdded(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getHardButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemAdded(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeAdded(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getSoftButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemAdded(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

bool AddActivityButtonCommand::valid() const
{
  return isValid;
}

}
}
