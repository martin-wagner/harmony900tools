// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <functional>

#include "base.h"
#include "document/data/items/state.h"

namespace document
{
namespace data
{

class AddDeviceActionSequenceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //add item to sm action
    //pos -1 = append
    //device -- discrete action
    AddDeviceActionSequenceCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateTransitionAction t, int seqPos = -1, QUndoCommand *parent = nullptr);

    //add item to num action
    AddDeviceActionSequenceCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, uint32_t digit, int seqPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    int seqPos;
    std::function<std::vector<item::SequenceItem>&()> getSeq;
};

}
}
