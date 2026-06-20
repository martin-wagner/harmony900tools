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

enum class StateTransitionType {
    Unknown,
    Discrete,
    Relative
};

enum class StateTransitionAction {
    Unknown,
    Discrete_Enter,
    Relative_Reset,
    Relative_Next,
    Relative_Prev
};

/** store info for a state machine.
 *
 * can be power, input, ... so essentially everything on a device that has
 * multiple states.
 */
class StateMachine
{
  public:
    PropertyEnum<StateMachineType> smType{StateMachineType::Power, Include::ALWAYS};
    PropertyU32 delayMs{100, Include::CHECK};

    DiscreteActions discrete;
    RelativeActions relative;
};

}
}
}
