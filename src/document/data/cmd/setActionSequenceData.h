// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetActionOpCommand: public SetPropertyBaseCommand<Enum<Operation>>
{
  public:
    SetActionOpCommand(ConfigData &c, const Enum<Operation> &value,
        uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
        uint32_t actPos, uint32_t seqPos, QUndoCommand *parent = nullptr);
};

class SetActionCmdCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetActionCmdCommand(ConfigData &c, const std::string &value,
        uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
        uint32_t actPos, uint32_t seqPos, QUndoCommand *parent = nullptr);
};

class SetActionDelayMsCommand: public SetPropertyBaseCommand<uint32_t>
{
  public:
    SetActionDelayMsCommand(ConfigData &c, uint32_t value,
        uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
        uint32_t actPos, uint32_t seqPos, QUndoCommand *parent = nullptr);
};

class SetActionStateNameCommand: public SetPropertyBaseCommand<Enum<StateMachineType>>
{
  public:
    SetActionStateNameCommand(ConfigData &c, const Enum<StateMachineType> &value,
        uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
        uint32_t actPos, uint32_t seqPos, QUndoCommand *parent = nullptr);
};

class SetActionStateValueCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetActionStateValueCommand(ConfigData &c, const std::string &value,
        uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
        uint32_t actPos, uint32_t seqPos, QUndoCommand *parent = nullptr);
};

class SetActionModCommand: public SetPropertyBaseCommand<Enum<Modifier>>
{
  public:
    SetActionModCommand(ConfigData &c, const Enum<Modifier> &value,
        uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
        uint32_t actPos, uint32_t seqPos, QUndoCommand *parent = nullptr);
};

}
}
