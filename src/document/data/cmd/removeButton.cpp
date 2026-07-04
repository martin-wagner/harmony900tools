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

RemoveButtonCommand::RemoveButtonCommand(ConfigData &c, uint32_t devicePos,
    uint32_t buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Button (Pos: %1)").arg(devicePos), parent), c(
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

void RemoveButtonCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();

  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.erase(buttons.begin() + buttonPos);
  emit dirtyChanged(true);
}

void RemoveButtonCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.insert(buttons.begin() + buttonPos, button);
  QUndoCommand::undo();
  emit dirtyChanged(true);
}

bool RemoveButtonCommand::valid() const
{
  return isValid;
}

}
}
