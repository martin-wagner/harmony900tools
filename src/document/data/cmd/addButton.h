// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/button.h"

namespace document
{
namespace data
{

class AddButtonCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    AddButtonCommand(ConfigData &c, item::ButtonType t, uint32_t devicePos, int buttonPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    item::ButtonType type;
    uint32_t devicePos;
    int buttonPos = -1;
};

}
}
