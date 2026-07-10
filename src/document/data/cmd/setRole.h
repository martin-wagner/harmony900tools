// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/commands.h"

namespace document
{
namespace data
{

class SetRoleCommand: public BaseCommand
{
    Q_OBJECT
  public:
    // rolePos -1 = append (insert mode only).
    // overwrite: replace existing item at rolePos; insert: shift items at rolePos.
    SetRoleCommand(ConfigData &c, uint32_t activityPos, item::Role &role, int rolePos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    bool overwrite = false;

    ConfigData &c;
    uint32_t activityPos;
    int rolePos = -1;
    item::Role role;
    item::Role prevRole;
};

}
}
