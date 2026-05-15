// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <map>
#include <chrono>

#include "document/domain/enum.h"
#include "action.h"
#include "unknown.h"

namespace document
{
namespace domain
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
    const std::chrono::milliseconds& getDelay() const
    {
      return delay;
    }

    void setDelay(const std::chrono::milliseconds &delay)
    {
      this->delay = delay;
    }

    const std::map<std::string, DeviceAction>& getDiscreteActions() const
    {
      return discreteActions;
    }

    void setDiscreteActions(
        const std::map<std::string, DeviceAction> &discreteActions)
    {
      this->discreteActions = discreteActions;
    }

    const Enum<StateMachineType>& getId() const
    {
      return id;
    }

    void setId(const Enum<StateMachineType> &id)
    {
      this->id = id;
    }

    const std::map<std::string, DeviceAction>& getRelativeActions() const
    {
      return relativeActions;
    }

    void setRelativeActions(
        const std::map<std::string, DeviceAction> &relativeActions)
    {
      this->relativeActions = relativeActions;
    }

    const std::vector<UnknownElement>& getUnknownItems() const
    {
      return u;
    }

    void setUnknownItems(const std::vector<UnknownElement> &u)
    {
      this->u = u;
    }

  protected:
    Enum<StateMachineType> id { StateMachineType::Power };
    std::chrono::milliseconds delay;
    std::map<std::string, DeviceAction> discreteActions;
    std::map<std::string, DeviceAction> relativeActions;

    std::vector<UnknownElement> u;
};

}
}
}
