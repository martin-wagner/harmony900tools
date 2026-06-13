// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetStatemachineTypeCommand: public SetPropertyBaseCommand<Enum<StateMachineType>>
{
  public:
    SetStatemachineTypeCommand(ConfigData &c, const Enum<StateMachineType> &value,
        uint32_t devicePos, int smPos,
        QUndoCommand *parent = nullptr);
};

class SetStatemachineDelayCommand: public SetPropertyBaseCommand<uint32_t>
{
  public:
    SetStatemachineDelayCommand(ConfigData &c, uint32_t value,
        uint32_t devicePos, int smPos, QUndoCommand *parent = nullptr);
};


class SetStatemachineActionClassCommand: public SetPropertyBaseCommand<Enum<ActionClass>>
{
  public:
    SetStatemachineActionClassCommand(ConfigData &c, const Enum<ActionClass> &value,
        uint32_t devicePos, uint32_t smPos,
        QUndoCommand *parent = nullptr);
};


}
}
