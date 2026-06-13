// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/state.h"

namespace document
{
namespace data
{

class AddDeviceActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    AddDeviceActionCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, int actPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    int actPos;
};

}
}
