// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>

#include "document/data/enum.h"
#include "action.h"
#include "unknown.h"

namespace document
{
namespace data
{
namespace item
{

/** simple state machine -- transition between every state at every time */
class DiscreteActions
{
  public:
    std::vector<std::string> states; //state[i[ corresponds to action[i]
    std::vector<DeviceAction> enterStateAction;

    const bool empty() const { return states.empty(); };
};

/** sequential state machine -- transitions to the next state on every
 * event. optionally has a reset action */
class RelativeActions
{
  public:
    std::optional<DeviceAction> resetAction = std::nullopt;
    std::vector<std::string> states; //each state transition calls next/prev
    std::optional<DeviceAction> nextStateAction = std::nullopt;
    std::optional<DeviceAction> prevStateAction = std::nullopt;

    const bool empty() const { return states.empty(); };
};

//fixme: usr/local/share/lua/5.1/ethanol/objects/State.lua
//line 64 -- somehow also supports MinValue and MaxValue as alternative to Value. Don't have an example how that looks...
//line 80 -- InitialValue -- same as first "Value"?

enum class StateMachineType {
    Unknown,
    Discrete,
    Relative
};

enum class StateMachineAction {
    Unknown,
    Start,
    Finish,
    Discrete_Enter,
    Relative_Reset,
    Relative_Next,
    Relative_Prev
};

/** store info for a state machine.
 *
 * can be power, input, ... so essentially everything on a device that has
 * multiple states.
 *
 * some parsing in usr/local/share/lua/5.1/ethanol/objects/State.lua
 */
class StateMachine
{
  public:
    PropertyEnum<StateMachineDeviceType> smType{StateMachineDeviceType::Power, Include::ALWAYS};
    PropertyU32 delayMs{100, Include::CHECK};

    std::optional<DeviceAction> startAction = std::nullopt;
    std::optional<DeviceAction> finishAction = std::nullopt;

    DiscreteActions discrete;
    RelativeActions relative;
};

}
}
}
