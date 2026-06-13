// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetDeviceActionTypeCommand: public SetPropertyBaseCommand<Enum<ActionType>>
{
  public:
    SetDeviceActionTypeCommand(ConfigData &c, const Enum<ActionType> &value,
        uint32_t devicePos, uint32_t smPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceActionNameCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetDeviceActionNameCommand(ConfigData &c, const std::string &value,
        uint32_t devicePos, uint32_t smPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceActionRepeatWillNotHarmCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceActionRepeatWillNotHarmCommand(ConfigData &c, const bool &value,
        uint32_t devicePos, uint32_t smPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};


class SetDeviceActionOpCommand: public SetPropertyBaseCommand<Enum<Operation>>
{
  public:
    SetDeviceActionOpCommand(ConfigData &c, const Enum<Operation> &value,
        uint32_t devicePos, uint32_t smPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};


class SetDeviceActionCmdCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetDeviceActionCmdCommand(ConfigData &c, const std::string &value,
        uint32_t devicePos, uint32_t smPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceActionDelayMsCommand: public SetPropertyBaseCommand<uint32_t>
{
  public:
    SetDeviceActionDelayMsCommand(ConfigData &c, const uint32_t &value,
        uint32_t devicePos, int buttonPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceActionStateNameCommand: public SetPropertyBaseCommand<Enum<StateMachineType>>
{
  public:
    SetDeviceActionStateNameCommand(ConfigData &c, const Enum<StateMachineType> &value,
        uint32_t devicePos, int buttonPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceActionStateValueCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetDeviceActionStateValueCommand(ConfigData &c, const std::string &value,
        uint32_t devicePos, int buttonPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceActionModCommand: public SetPropertyBaseCommand<Enum<Modifier>>
{
  public:
    SetDeviceActionModCommand(ConfigData &c, const Enum<Modifier> &value,
        uint32_t devicePos, uint32_t smPos, uint32_t actPos,
        QUndoCommand *parent = nullptr);
};










}
}
