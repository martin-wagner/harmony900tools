// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>

#include "document/data/enum.h"
#include "document/data/property.h"
#include "unknown.h"

namespace document
{
namespace data
{
namespace item
{

/** a action can require multiple "steps" to complete
 * dispatcher: usr/local/share/lua/5.1/ethanol/ethanol line 113 handleAction
 * */
class SequenceItem {
  public:
    PropertyEnum<Operation> opcode{Operation::SendCommand, Used::YES};
    PropertyString cmd{"", Used::NO};
    PropertyU32 deviceId{0, Used::NO}; //device -- not needed (id of parent, redundant); activity -- referenced device
    PropertyU32 delayMs{1000, Used::NO};
    PropertyEnum<StateMachineDeviceType> stateName{StateMachineDeviceType::Unknown, Used::NO};
    PropertyString stateValue{"", Used::NO};
    PropertyEnum<Modifier> mod{Modifier::Press, Used::NO};

    const std::vector<UnknownElement> &getUnknownParams() const
    {
      return unknownParams;
    }

    std::vector<UnknownElement> &getUnknownParams()
    {
      return unknownParams;
    }

  protected:
    std::vector<UnknownElement> unknownParams;
};

/** one device action (state machine, activity change, number buttons) */
class DeviceAction
{
  public:
    PropertyEnum<ActionType> actionType{ActionType::None, Used::NO};
    PropertyBool repeatWillNotHarm{false, Used::YES};

    std::vector<SequenceItem> sequence;
};

}
}
}
