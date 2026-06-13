// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addButton.h"

using namespace std;

namespace document
{
namespace data
{

AddButtonCommand::AddButtonCommand(ConfigData &c, item::ButtonType t,
    uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Button (to device: %1)").arg(devicePos),
        parent), c(c), type(t), devicePos(devicePos)
{
  uint32_t buttonCount;

  if (devicePos >= c.getDevices().size()) {
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

void AddButtonCommand::redo()
{
  if (!isValid) {
    return;
  }
  switch (type) {
    case item::ButtonType::Hard: {
      auto &buttons = c.getDevices()[devicePos].getHardButtons();
      buttons.insert(buttons.begin() + buttonPos, item::Button(type));
      break;
    }
    case item::ButtonType::Soft: {
      auto &buttons = c.getDevices()[devicePos].getSoftButtons();
      buttons.insert(buttons.begin() + buttonPos, item::Button(type));
      break;
    }
    default:
      return;
  }
  emit dirtyChanged(true);
}

void AddButtonCommand::undo()
{
  if (!isValid) {
    return;
  }
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

bool AddButtonCommand::valid() const
{
  return isValid;
}

}
}
