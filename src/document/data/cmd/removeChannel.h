// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveActivityChannelCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveActivityChannelCommand(ConfigData &c, uint32_t devicePos, uint32_t channelPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t channelPos;
    item::Channel channel;
};



}
}
