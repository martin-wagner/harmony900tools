// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeButton.h"

using namespace std;

namespace document
{
namespace data
{

RemoveButtonFromActivityCommand::RemoveButtonFromActivityCommand(
    const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove button from activity"), parent)
{
  // todo implement this!
}

void RemoveButtonFromActivityCommand::redo()
{
}

void RemoveButtonFromActivityCommand::undo()
{
}

RemoveDeviceButtonCommand::RemoveDeviceButtonCommand(ConfigData &c, uint32_t devicePos,
    uint32_t buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Button (from device: %1)").arg(devicePos), parent), c(
        c), devicePos(devicePos), buttonPos(buttonPos), button(
        item::ButtonType::Hard)
{
  uint32_t buttonCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }
  buttonCount = c.getDevices()[devicePos].getButtons().size();
  if (buttonPos >= buttonCount) {
    return;
  }

  // copy Button for undo
  button = c.getDevices()[devicePos].getButtons()[buttonPos];
  // add activities for undo
  for (auto &a : c.getActivities()) {
//    auto *cmd = new RemoveButtonFromActivityCommand(ids, a, this);
//
//    connect(cmd, &RemoveButtonFromActivityCommand::writeLog, this,
//        &RemoveButtonCommand::writeLog);
//    connect(cmd, &RemoveButtonFromActivityCommand::writeMsg, this,
//        &RemoveButtonCommand::writeMsg);
//    connect(cmd, &RemoveButtonFromActivityCommand::activityAboutToBeAdded, this,
//        &RemoveButtonCommand::activityAboutToBeAdded);
//    connect(cmd, &RemoveButtonFromActivityCommand::activityAdded, this,
//        &RemoveButtonCommand::activityAdded);
//    connect(cmd, &RemoveButtonFromActivityCommand::activityAboutToBeRemoved,
//        this, &RemoveButtonCommand::activityAboutToBeRemoved);
//    connect(cmd, &RemoveButtonFromActivityCommand::activityRemoved, this,
//        &RemoveButtonCommand::activityRemoved); todo
  }
  isValid = true;
}

void RemoveDeviceButtonCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_BUTTON, buttonPos);
  QUndoCommand::redo();
  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.erase(buttons.begin() + buttonPos);
  emit itemRemoved(Item::DEVICE_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

void RemoveDeviceButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_BUTTON, buttonPos);
  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.insert(buttons.begin() + buttonPos, button);
  QUndoCommand::undo();
  emit itemAdded(Item::DEVICE_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

bool RemoveDeviceButtonCommand::valid() const
{
  return isValid;
}

RemoveActivityButtonCommand::RemoveActivityButtonCommand(ConfigData &c, uint32_t activityPos,
    uint32_t buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Button (from activity: %1)").arg(activityPos), parent), c(
        c), activityPos(activityPos), buttonPos(buttonPos), button(
        item::ButtonType::Hard)
{
  uint32_t buttonCount;

  if (activityPos >= c.getActivities().size()) {
    //beyond end
    return;
  }
  buttonCount = c.getActivities()[activityPos].getButtons().size();
  if (buttonPos >= buttonCount) {
    return;
  }

  // copy Button for undo
  button = c.getActivities()[activityPos].getButtons()[buttonPos];
  isValid = true;
}

void RemoveActivityButtonCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY_BUTTON, buttonPos);
  QUndoCommand::redo();
  auto &buttons = c.getActivities()[activityPos].getButtons();
  buttons.erase(buttons.begin() + buttonPos);
  emit itemRemoved(Item::ACTIVITY_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

void RemoveActivityButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_BUTTON, buttonPos);
  auto &buttons = c.getActivities()[activityPos].getButtons();
  buttons.insert(buttons.begin() + buttonPos, button);
  QUndoCommand::undo();
  emit itemAdded(Item::ACTIVITY_BUTTON, buttonPos);
  emit dirtyChanged(true);
}

bool RemoveActivityButtonCommand::valid() const
{
  return isValid;
}

}
}
