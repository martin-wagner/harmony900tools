// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeNumpad.h"

using namespace std;

namespace document
{
namespace data
{

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
  if (optPad == nullopt) {
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
  c.getDevices()[devicePos].getNumpad() = nullopt;
  emit dirtyChanged(true);
}

void RemoveNumpadCommand::undo()
{
  if (!isValid) {
    return;
  }
  c.getDevices()[devicePos].getNumpad() = pad;
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
    case item::DigitSection::Finish:
      c.getDevices()[devicePos].getNumpad()->finish = nullopt;
      break;
    default:
      return;
  }
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
    case item::DigitSection::Finish:
      c.getDevices()[devicePos].getNumpad()->finish = a;
      break;
    default:
      return;
  }
  emit dirtyChanged(true);
}

bool RemoveDigitsCommand::valid() const
{
  return isValid;
}

}
}
