// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/state.h"
#include <functional>

namespace document
{
namespace data
{

class RemoveDeviceSmActionSequenceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //discrete action
    RemoveDeviceSmActionSequenceCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateTransitionAction t, uint32_t seqPos, QUndoCommand *parent = nullptr);

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
