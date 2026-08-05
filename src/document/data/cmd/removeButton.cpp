// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeButton.h"

using namespace std;

namespace document
{
namespace data
{

RemoveDeviceButtonCommand::RemoveDeviceButtonCommand(ConfigData &c,
    uint32_t devicePos, item::ButtonType t, uint32_t buttonPos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Button (from device: %1)").arg(devicePos),
        parent), c(c), type(t), devicePos(devicePos), buttonPos(buttonPos)
{
  try {
    // copy Button for undo
    switch (type) {
      case item::ButtonType::Hard:
        button = c.getDevices().at(devicePos).getHardButtons().at(buttonPos);
        break;
      case item::ButtonType::Soft:
        button = c.getDevices().at(devicePos).getSoftButtons().at(buttonPos);
        break;
      default:
        return;
    }
    isValid = true;

  } catch (std::out_of_range&) {
  }
}

void RemoveDeviceButtonCommand::redo()
{
  if (!isValid) {
    return;
  }

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeRemoved(Item::DEVICE_HARD_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getHardButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemRemoved(Item::DEVICE_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeRemoved(Item::DEVICE_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getSoftButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemRemoved(Item::DEVICE_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

void RemoveDeviceButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeAdded(Item::DEVICE_HARD_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getHardButtons();
      buttons.insert(buttons.begin() + buttonPos, button);
      emit itemAdded(Item::DEVICE_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeAdded(Item::DEVICE_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getDevices()[devicePos].getSoftButtons();
      buttons.insert(buttons.begin() + buttonPos, button);
      emit itemAdded(Item::DEVICE_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

bool RemoveDeviceButtonCommand::valid() const
{
  return isValid;
}

RemoveActivityButtonCommand::RemoveActivityButtonCommand(ConfigData &c,
    uint32_t activityPos, item::ButtonType t, uint32_t buttonPos,
    QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Button (from activity: %1)").arg(activityPos),
        parent), c(c), type(t), activityPos(activityPos), buttonPos(buttonPos)
{
  try {
    // copy Button for undo
    switch (type) {
      case item::ButtonType::Hard:
        button = c.getActivities().at(activityPos).getHardButtons().at(
            buttonPos);
        break;
      case item::ButtonType::Soft:
        button = c.getActivities().at(activityPos).getSoftButtons().at(
            buttonPos);
        break;
      default:
        return;
    }
    isValid = true;

  } catch (std::out_of_range&) {
  }
}

void RemoveActivityButtonCommand::redo()
{
  if (!isValid) {
    return;
  }

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeRemoved(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getHardButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemRemoved(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeRemoved(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getSoftButtons();
      buttons.erase(buttons.begin() + buttonPos);
      emit itemRemoved(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

void RemoveActivityButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  switch (type) {
    case item::ButtonType::Hard: {
      emit itemAboutToBeAdded(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getHardButtons();
      buttons.insert(buttons.begin() + buttonPos, button);
      emit itemAdded(Item::ACTIVITY_HARD_BUTTON, buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      emit itemAboutToBeAdded(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      auto &buttons = c.getActivities()[activityPos].getSoftButtons();
      buttons.insert(buttons.begin() + buttonPos, button);
      emit itemAdded(Item::ACTIVITY_SOFT_BUTTON, buttonPos);
      break;
    }
    default:
      break;
  }
  emit dirtyChanged(true);
}

bool RemoveActivityButtonCommand::valid() const
{
  return isValid;
}

}
}
