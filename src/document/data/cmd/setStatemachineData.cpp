// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setStatemachineData.h"

using namespace std;

namespace document
{
namespace data
{

SetStatemachineTypeCommand::SetStatemachineTypeCommand(ConfigData &c,
    const Enum<StateMachineType> &value, uint32_t devicePos, int smPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<Enum<StateMachineType>>(QObject::tr("set state machine type"),
        [&c, devicePos, smPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].smType.get();
        },
        [&c, devicePos, smPos](const Enum<StateMachineType> &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].smType.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetStatemachineDelayCommand::SetStatemachineDelayCommand(ConfigData &c,
    uint32_t value, uint32_t devicePos, int smPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<uint32_t>(QObject::tr("set state machine delay"),
        [&c, devicePos, smPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].delayMs.get();
        },
        [&c, devicePos, smPos](const uint32_t &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].delayMs.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetStatemachineActionClassCommand::SetStatemachineActionClassCommand(ConfigData &c,
    const Enum<ActionClass> &value, uint32_t devicePos, uint32_t smPos,
    QUndoCommand *parent):
    SetPropertyBaseCommand<Enum<ActionClass>>(QObject::tr("set device action class"),
        [&c, devicePos, smPos]() {
          return c.getDevices()[devicePos].getStateMachines()[smPos].actionClass.get();
        },
        [&c, devicePos, smPos](const Enum<ActionClass> &v) {
          c.getDevices()[devicePos].getStateMachines()[smPos].actionClass.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

}
}
