// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveNumpadCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveNumpadCommand(ConfigData &c, uint32_t devicePos,  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    item::Numpad pad;
};

class RemoveDigitsCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDigitsCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    item::DigitSection s;
    item::Digits d;
    item::DeviceAction a;
};

}
}
