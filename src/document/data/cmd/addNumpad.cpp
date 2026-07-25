// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addNumpad.h"

using namespace std;

namespace document
{
namespace data
{

AddNumpadCommand::AddNumpadCommand(ConfigData &c, uint32_t devicePos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Numpad (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos)
{
  if (devicePos >= c.getDevices().size()) {
    return;
  }
  isValid = true;
}

void AddNumpadCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_NUMPAD, 0);
  c.getDevices()[devicePos].getNumpad() = item::Numpad();
  emit itemAdded(Item::DEVICE_NUMPAD, 0);
  emit dirtyChanged(true);
}

void AddNumpadCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_NUMPAD, 0);
  c.getDevices()[devicePos].getNumpad() = nullopt;
  emit itemRemoved(Item::DEVICE_NUMPAD, 0);
  emit dirtyChanged(true);
}

bool AddNumpadCommand::valid() const
{
  return isValid;
}

AddDigitsCommand::AddDigitsCommand(ConfigData &c, uint32_t devicePos,
    item::DigitSection s, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Add Section to Numpad (to device: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), s(s)
{
  if (devicePos >= c.getDevices().size()) {
    return;
  }
  isValid = true;
}

void AddDigitsCommand::redo()
{
  if (!isValid) {
    return;
  }

  switch (s) {
    case item::DigitSection::First:
      c.getDevices()[devicePos].getNumpad()->first = item::Digits();
      break;
    case item::DigitSection::Middle:
      c.getDevices()[devicePos].getNumpad()->middle = item::Digits();
      break;
    case item::DigitSection::Last:
      c.getDevices()[devicePos].getNumpad()->last = item::Digits();
      break;
    case item::DigitSection::Finish:
      c.getDevices()[devicePos].getNumpad()->finish = item::DeviceAction();
      break;
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_NUMPAD, 0);
  emit dirtyChanged(true);
}

void AddDigitsCommand::undo()
{
  if (!isValid) {
    return;
  }

  switch (s) {
    case item::DigitSection::First:
      c.getDevices()[devicePos].getNumpad()->first = nullopt;
      break;
    case item::DigitSection::Middle:
      c.getDevices()[devicePos].getNumpad()->middle = nullopt;
      break;
    case item::DigitSection::Last:
      c.getDevices()[devicePos].getNumpad()->last = nullopt;
      break;
    case item::DigitSection::Finish:
      c.getDevices()[devicePos].getNumpad()->finish = nullopt;
      break;
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_NUMPAD, 0);
  emit dirtyChanged(true);
}

bool AddDigitsCommand::valid() const
{
  return isValid;
}

}
}
