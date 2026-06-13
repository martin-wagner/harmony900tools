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

/** one device action (state machine, activity change, number buttons)
 *
 * todo -- geht immer nur eine operation?? */
class DeviceAction
{
  public:
    PropertyEnum<ActionType> actionType{ActionType::None, Include::ALWAYS};
    PropertyString name{"", Include::ALWAYS};
    PropertyBool repeatWillNotHarm{false, Include::ALWAYS};
    PropertyEnum<Operation> op{Operation::SendCommnad, Include::ALWAYS};
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

/** one state switch action */
class StateAction
{
  public:
    PropertyEnum<StateMachineType> action{StateMachineType::Power, Include::ALWAYS};
    PropertyString deviceAction{"", Include::ALWAYS};
};

}
}
}
