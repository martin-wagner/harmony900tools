// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/button.h"

namespace document
{
namespace data
{

class AddActivityChannelCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    AddActivityChannelCommand(ConfigData &c, uint32_t activityPos, int channelPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t activityPos;
    int channelPos = -1;
};


}
}
