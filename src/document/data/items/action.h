// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>

#include "document/data/enum.h"
#include "document/data/property.h"

namespace document
{
namespace data
{
namespace item
{

/** a action can require multiple "steps" to complete */
class SequenceItem {
  public:
    PropertyEnum<Operation> opcode{Operation::SendCommand, Include::ALWAYS};
    PropertyString cmd{"", Include::CHECK};
    PropertyU32 delayMs{1000, Include::CHECK};
    PropertyEnum<StateMachineType> stateName{StateMachineType::Unknown, Include::CHECK};
    PropertyString stateValue{"", Include::CHECK};
    PropertyEnum<Modifier> mod{Modifier::Press, Include::CHECK};

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
    PropertyEnum<ActionType> actionType{ActionType::None, Include::CHECK};
    PropertyBool repeatWillNotHarm{false, Include::ALWAYS};

    std::vector<SequenceItem> sequence;
};

}
}
}
