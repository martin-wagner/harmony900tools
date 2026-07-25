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
    RemoveDeviceActionSequenceCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateMachineAction t, uint32_t seqPos, QUndoCommand *parent = nullptr);

    //remove item from num action
    RemoveDeviceActionSequenceCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos, QUndoCommand *parent = nullptr);

    //add item to activity action
    RemoveDeviceActionSequenceCommand(ConfigData &c, uint32_t activityPos, item::ActivityAction t, uint32_t actionPos, uint32_t seqPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    uint32_t seqPos;
    item::SequenceItem sequenceItem;
    std::function<std::vector<item::SequenceItem>&()> getSeq;
    Item item;
    uint32_t itemPos;
};

}
}
