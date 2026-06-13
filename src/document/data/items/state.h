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

/** store info for a state machine.
 *
 * can be power, input, ... so essentially everything on a device that has
 * multiple states.
 */
class StateMachine
{
  public:
    const std::vector<DeviceAction>& getActions() const
    {
      return actions;
    }

    std::vector<DeviceAction>& getActions()
    {
      return actions;
    }

    PropertyEnum<StateMachineType> smType{StateMachineType::Power, Include::ALWAYS};
    PropertyU32 delayMs{100, Include::CHECK};
    PropertyEnum<ActionClass> actionClass{ActionClass::DiscreteActions, Include::ALWAYS};

  protected:
    //actual type defined by actionclass
    std::vector<DeviceAction> actions;
};

}
}
}
