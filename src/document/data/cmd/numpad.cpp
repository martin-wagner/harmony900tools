// SPDX-License-Identifier: LGPL-2.1-or-later

#include "getActionData.h"
#include "numpad.h"

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
    case item::DigitSection::Start:
      c.getDevices()[devicePos].getNumpad()->start = item::DeviceAction();
      break;
    case item::DigitSection::GreaterTen:
      c.getDevices()[devicePos].getNumpad()->greaterTen = item::DeviceAction();
      break;
    case item::DigitSection::GreaterHundred:
      c.getDevices()[devicePos].getNumpad()->greaterHundred = item::DeviceAction();
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
    case item::DigitSection::Start:
      c.getDevices()[devicePos].getNumpad()->start = nullopt;
      break;
    case item::DigitSection::GreaterTen:
      c.getDevices()[devicePos].getNumpad()->greaterTen = nullopt;
      break;
    case item::DigitSection::GreaterHundred:
      c.getDevices()[devicePos].getNumpad()->greaterHundred = nullopt;
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



RemoveNumpadCommand::RemoveNumpadCommand(ConfigData &c, uint32_t devicePos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Numpad (Pos: %1)").arg(devicePos), parent), c(
        c), devicePos(devicePos)
{
  if (devicePos >= c.getDevices().size()) {
    //beyond end
    return;
  }
  auto &optPad = c.getDevices()[devicePos].getNumpad();
  if (optPad.has_value()) {
    return;
  }
  pad = *(c.getDevices()[devicePos].getNumpad());
  isValid = true;
}

void RemoveNumpadCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE_NUMPAD, 0);
  c.getDevices()[devicePos].getNumpad() = nullopt;
  emit itemRemoved(Item::DEVICE_NUMPAD, 0);
  emit dirtyChanged(true);
}

void RemoveNumpadCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::DEVICE_NUMPAD, 0);
  c.getDevices()[devicePos].getNumpad() = pad;
  emit itemAdded(Item::DEVICE_NUMPAD, 0);
  emit dirtyChanged(true);
}

bool RemoveNumpadCommand::valid() const
{
  return isValid;
}

RemoveDigitsCommand::RemoveDigitsCommand(ConfigData &c, uint32_t devicePos,
    item::DigitSection s, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Section from Numpad (Pos: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), s(s)
{
  try {
    switch (s) {
      case item::DigitSection::First:
        d = c.getDevices().at(devicePos).getNumpad()->first.value();
        break;
      case item::DigitSection::Middle:
        d = c.getDevices().at(devicePos).getNumpad()->middle.value();
        break;
      case item::DigitSection::Last:
        d = c.getDevices().at(devicePos).getNumpad()->last.value();
        break;
      case item::DigitSection::Start:
        a = c.getDevices().at(devicePos).getNumpad()->start.value();
        break;
      case item::DigitSection::GreaterTen:
        a = c.getDevices().at(devicePos).getNumpad()->greaterTen.value();
        break;
      case item::DigitSection::GreaterHundred:
        a = c.getDevices().at(devicePos).getNumpad()->greaterHundred.value();
        break;
      case item::DigitSection::Finish:
        a = c.getDevices().at(devicePos).getNumpad()->finish.value();
        break;
      default:
        return;
    }
  } catch (const exception&) {
    return;
  }
  isValid = true;
}

void RemoveDigitsCommand::redo()
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
    case item::DigitSection::Start:
      c.getDevices()[devicePos].getNumpad()->start = nullopt;
      break;
    case item::DigitSection::GreaterTen:
      c.getDevices()[devicePos].getNumpad()->greaterTen = nullopt;
      break;
    case item::DigitSection::GreaterHundred:
      c.getDevices()[devicePos].getNumpad()->greaterHundred = nullopt;
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

void RemoveDigitsCommand::undo()
{
  if (!isValid) {
    return;
  }

  switch (s) {
    case item::DigitSection::First:
      c.getDevices()[devicePos].getNumpad()->first = d;
      break;
    case item::DigitSection::Middle:
      c.getDevices()[devicePos].getNumpad()->middle = d;
      break;
    case item::DigitSection::Last:
      c.getDevices()[devicePos].getNumpad()->last = d;
      break;
    case item::DigitSection::Start:
      c.getDevices()[devicePos].getNumpad()->start = a;
      break;
    case item::DigitSection::GreaterTen:
      c.getDevices()[devicePos].getNumpad()->greaterTen = a;
      break;
    case item::DigitSection::GreaterHundred:
      c.getDevices()[devicePos].getNumpad()->greaterHundred = a;
      break;
    case item::DigitSection::Finish:
      c.getDevices()[devicePos].getNumpad()->finish = a;
      break;
    default:
      return;
  }
  emit itemChanged(Item::DEVICE_NUMPAD, 0);
  emit dirtyChanged(true);
}

bool RemoveDigitsCommand::valid() const
{
  return isValid;
}



SetNumpadActionCommand::SetNumpadActionCommand(ConfigData &c,
    uint32_t devicePos, item::DigitSection s, uint32_t digit,
    const item::DeviceAction &act, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Edit Numpad action"), parent), c(c), devicePos(
        devicePos), s(s), digit(digit), act(act)
{
  auto *action = getActionFromNumpadRef(c, devicePos, s, digit);
  if (action == nullptr) {
    return;
  }
  oldAct = *action;

  isValid = true;
}
void SetNumpadActionCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto *action = getActionFromNumpadRef(c, devicePos, s, digit);
  *action = act;
  emit itemChanged(Item::DEVICE_NUMPAD, devicePos);
  emit dirtyChanged(true);
}
void SetNumpadActionCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto *action = getActionFromNumpadRef(c, devicePos, s, digit);
  *action = oldAct;
  emit itemChanged(Item::DEVICE_NUMPAD, devicePos);
  emit dirtyChanged(true);
}
bool SetNumpadActionCommand::valid() const
{
  return isValid;
}

}
}
