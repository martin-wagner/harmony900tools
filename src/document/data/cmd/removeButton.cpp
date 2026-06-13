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

RemoveButtonCommand::RemoveButtonCommand(ConfigData &c, item::ButtonType t,
    uint32_t devicePos, uint32_t buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Button (Pos: %1)").arg(devicePos), parent), c(
        c), type(t), devicePos(devicePos), buttonPos(buttonPos), button(
        item::ButtonType::Hard)
{
  uint32_t buttonCount;

  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }
  switch (t) {
    case item::ButtonType::Hard:
      buttonCount = c.getDevices()[devicePos].getHardButtons().size();
      break;
    case item::ButtonType::Soft:
      buttonCount = c.getDevices()[devicePos].getSoftButtons().size();
      break;
    default:
      return;
  }
  if (buttonPos >= buttonCount) {
    return;
  }

  // copy Button for undo
  switch (t) {
    case item::ButtonType::Hard:
      button = c.getDevices()[devicePos].getHardButtons()[buttonPos];
      break;
    case item::ButtonType::Soft:
      button = c.getDevices()[devicePos].getSoftButtons()[buttonPos];
      break;
    default:
      return;
  }
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

void RemoveButtonCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();

  switch (type) {
    case item::ButtonType::Hard: {
      auto &buttons = c.getDevices()[devicePos].getHardButtons();
      buttons.erase(buttons.begin() + buttonPos);
      break;
    }
    case item::ButtonType::Soft: {
      auto &buttons = c.getDevices()[devicePos].getSoftButtons();
      buttons.erase(buttons.begin() + buttonPos);
      break;
    }
    default:
      return;
  }
  emit dirtyChanged(true);
}

void RemoveButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  switch (type) {
    case item::ButtonType::Hard: {
      auto &buttons = c.getDevices()[devicePos].getHardButtons();
      buttons.insert(buttons.begin() + buttonPos, button);
      break;
    }
    case item::ButtonType::Soft: {
      auto &buttons = c.getDevices()[devicePos].getSoftButtons();
      buttons.insert(buttons.begin() + buttonPos, button);
      break;
    }
    default:
      return;
  }
  QUndoCommand::undo();
  emit dirtyChanged(true);
}

bool RemoveButtonCommand::valid() const
{
  return isValid;
}

}
}
