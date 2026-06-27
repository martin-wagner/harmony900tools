// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/commands.h"

namespace document
{
namespace data
{

class SetIrCommand: public BaseCommand
{
    Q_OBJECT
  public:
    // cmdPos -1 = append (insert mode only).
    // overwrite: replace existing item at cmdPos; insert: shift items at cmdPos.
    SetIrCommand(ConfigData &c, uint32_t devicePos, item::ProtoCommand &cmd, int cmdPos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);
    SetIrCommand(ConfigData &c, uint32_t devicePos, item::RawCommand &cmd, int cmdPos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    bool overwrite = false;

    ConfigData &c;
    uint32_t devicePos;
    int cmdPos = -1;
    std::optional<item::ProtoCommand> proto;
    std::optional<item::ProtoCommand> prevProto;
    std::optional<item::RawCommand> raw;
    std::optional<item::RawCommand> prevRaw;
};

}
}
