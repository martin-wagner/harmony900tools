// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/state.h"

namespace document
{
namespace data
{

class AddActivityActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddActivityActionCommand(ConfigData &c, uint32_t activityPos, item::ActivityAction t, int actionPos = -1,  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t activityPos;
    uint32_t actionPos;
    item::ActivityAction t;
};

}
}
