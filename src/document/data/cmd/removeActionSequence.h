// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/state.h"
#include <functional>

namespace document
{
namespace data
{

class RemoveDeviceActionSequenceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //remove item from sm action
    //device -- discrete action
    RemoveDeviceActionSequenceCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateTransitionAction t, uint32_t seqPos, QUndoCommand *parent = nullptr);

    //remove item from num action
    RemoveDeviceActionSequenceCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, uint32_t digit, int seqPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    uint32_t seqPos;
    item::SequenceItem sequenceItem;
    std::function<std::vector<item::SequenceItem>&()> getSeq;
};

}
}
