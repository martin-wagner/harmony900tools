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

void AddButtonCommand::redo()
{
  if (!isValid) {
    return;
  }
  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.insert(buttons.begin() + buttonPos, item::Button(type));
  emit dirtyChanged(true);
}

void AddButtonCommand::undo()
{
  if (!isValid) {
    return;
  }
  auto &buttons = c.getDevices()[devicePos].getButtons();
  buttons.erase(buttons.begin() + buttonPos);
  emit dirtyChanged(true);
}

bool AddButtonCommand::valid() const
{
  return isValid;
}

}
}
