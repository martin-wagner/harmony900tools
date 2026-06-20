// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/button.h"

namespace document
{
namespace data
{

class AddNumpadCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddNumpadCommand(ConfigData &c, uint32_t devicePos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
};

class AddDigitsCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddDigitsCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    item::DigitSection s;
};

}
}
