// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setActionSequenceData.h"
#include "setActionData.h"

using namespace std;

namespace document
{
namespace data
{

SetActionOpCommand::SetActionOpCommand(ConfigData &c, const Enum<Operation> &value,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<Operation>>(QObject::tr("set device action sequence opcode"),
        [&c, devicePos, smPos, actPos, t, seqPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].opcode.get();
        }, [&c, devicePos, smPos, actPos, t, seqPos](const Enum<Operation> &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].opcode.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetActionCmdCommand::SetActionCmdCommand(ConfigData &c, const std::string &value,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<std::string>(QObject::tr("set device action sequence cmd"),
        [&c, devicePos, smPos, actPos, t, seqPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].cmd.get();
        }, [&c, devicePos, smPos, actPos, t, seqPos](const std::string &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].cmd.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetActionDelayMsCommand::SetActionDelayMsCommand(ConfigData &c, uint32_t value,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<uint32_t>(QObject::tr("set device action sequence delay ms"),
        [&c, devicePos, smPos, actPos, t, seqPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].delayMs.get();
        }, [&c, devicePos, smPos, actPos, t, seqPos](const uint32_t &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].delayMs.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetActionStateNameCommand::SetActionStateNameCommand(ConfigData &c, const Enum<StateMachineType> &value,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<StateMachineType>>(QObject::tr("set device action sequence state name"),
        [&c, devicePos, smPos, actPos, t, seqPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateName.get();
        }, [&c, devicePos, smPos, actPos, t, seqPos](const Enum<StateMachineType> &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateName.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetActionStateValueCommand::SetActionStateValueCommand(ConfigData &c, const std::string &value,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<std::string>(QObject::tr("set device action sequence state value"),
        [&c, devicePos, smPos, actPos, t, seqPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateValue.get();
        }, [&c, devicePos, smPos, actPos, t, seqPos](const std::string &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].stateValue.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetActionModCommand::SetActionModCommand(ConfigData &c, const Enum<Modifier> &value,
    uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,
    uint32_t actPos, uint32_t seqPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<Modifier>>(QObject::tr("set device action sequence modifier"),
        [&c, devicePos, smPos, actPos, t, seqPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].mod.get();
        }, [&c, devicePos, smPos, actPos, t, seqPos](const Enum<Modifier> &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].mod.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

}
}
