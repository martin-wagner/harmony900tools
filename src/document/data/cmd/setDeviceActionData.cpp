// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setDeviceActionData.h"

using namespace std;

namespace document
{
namespace data
{

SetDeviceActionTypeCommand::SetDeviceActionTypeCommand(ConfigData &c,
    const Enum<ActionType> &value, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<Enum<ActionType>>(QObject::tr("set device action type"),
        [&c, devicePos, smPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].actionType.get();
        },
        [&c, devicePos, smPos, actPos](const Enum<ActionType> &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].actionType.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionNameCommand::SetDeviceActionNameCommand(ConfigData &c,
    const string &value, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<string>(QObject::tr("set device action name"),
        [&c, devicePos, smPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].name.get();
        },
        [&c, devicePos, smPos, actPos](const string &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].name.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionRepeatWillNotHarmCommand::SetDeviceActionRepeatWillNotHarmCommand(ConfigData &c,
    const bool &value, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<bool>(QObject::tr("set device action repeat will not harm"),
        [&c, devicePos, smPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].repeatWillNotHarm.get();
        },
        [&c, devicePos, smPos, actPos](const bool &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].repeatWillNotHarm.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionOpCommand::SetDeviceActionOpCommand(ConfigData &c,
    const Enum<Operation> &value, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<Enum<Operation>>(QObject::tr("set device action op"),
        [&c, devicePos, smPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].op.get();
        },
        [&c, devicePos, smPos, actPos](const Enum<Operation> &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].op.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionCmdCommand::SetDeviceActionCmdCommand(ConfigData &c,
    const string &value, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<string>(QObject::tr("set device action cmd"),
        [&c, devicePos, smPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].cmd.get();
        },
        [&c, devicePos, smPos, actPos](const string &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].cmd.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionDelayMsCommand::SetDeviceActionDelayMsCommand(ConfigData &c,
    const uint32_t &value, uint32_t devicePos, int buttonPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<uint32_t>(QObject::tr("set device action delay ms"),
        [&c, devicePos, buttonPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[buttonPos].getActions()[actPos].delayMs.get();
        },
        [&c, devicePos, buttonPos, actPos](const uint32_t &v) {
          c.getDevices()[devicePos].getStateMachines()[buttonPos].getActions()[actPos].delayMs.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionStateNameCommand::SetDeviceActionStateNameCommand(ConfigData &c,
    const Enum<StateMachineType> &value, uint32_t devicePos, int buttonPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<Enum<StateMachineType>>(QObject::tr("set device action state name"),
        [&c, devicePos, buttonPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[buttonPos].getActions()[actPos].stateName.get();
        },
        [&c, devicePos, buttonPos, actPos](const Enum<StateMachineType> &v) {
          c.getDevices()[devicePos].getStateMachines()[buttonPos].getActions()[actPos].stateName.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionStateValueCommand::SetDeviceActionStateValueCommand(ConfigData &c,
    const std::string &value, uint32_t devicePos, int buttonPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<std::string>(QObject::tr("set device action state value"),
        [&c, devicePos, buttonPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[buttonPos].getActions()[actPos].stateValue.get();
        },
        [&c, devicePos, buttonPos, actPos](const std::string &v) {
          c.getDevices()[devicePos].getStateMachines()[buttonPos].getActions()[actPos].stateValue.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceActionModCommand::SetDeviceActionModCommand(ConfigData &c,
    const Enum<Modifier> &value, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<Enum<Modifier>>(QObject::tr("set device action mod"),
        [&c, devicePos, smPos, actPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].mod.get();
        },
        [&c, devicePos, smPos, actPos](const Enum<Modifier> &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].getActions()[actPos].mod.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}


}
}
