// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <functional>

#include "base.h"
#include "document/data/items/state.h"

namespace document
{
namespace data
{

class AddDeviceSmActionSequenceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    //device -- discrete action
    AddDeviceSmActionSequenceCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateTransitionAction t, int seqPos = -1, QUndoCommand *parent = nullptr);

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
