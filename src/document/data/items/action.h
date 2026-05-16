// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>

#include "document/data/enum.h"

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
    DeviceAction(uint32_t id = 0, const std::string &cmd = "", bool canRepeat =
        false);

    const Enum<ActionType>& getAction() const
    {
      return action;
    }

    void setAction(const Enum<ActionType> &action)
    {
      this->action = action;
    }

    const Enum<Operation>& getOpCode() const
    {
      return op;
    }

    void setOpCode(const Enum<Operation> &op)
    {
      this->op = op;
    }

    const std::string& getCmd() const
    {
      return cmd;
    }

    void setCmd(const std::string &cmd)
    {
      this->cmd = cmd;
    }

    uint32_t getId() const
    {
      return id;
    }

    void setId(uint32_t id)
    {
      this->id = id;
    }

    const Enum<Modifier>& getMod() const
    {
      return mod;
    }

    void setMod(const Enum<Modifier> &mod)
    {
      this->mod = mod;
    }

    bool canRepeat() const
    {
      return repeatWillNotHarm;
    }

    void setRepeatWillNotHarm(bool repeatWillNotHarm)
    {
      this->repeatWillNotHarm = repeatWillNotHarm;
    }

  protected:
    Enum<ActionType> action { ActionType::None };
    Enum<Operation> op { Operation::SendCommnad };
    uint32_t id;
    std::string cmd;
    Enum<Modifier> mod { Modifier::Press };

    bool repeatWillNotHarm;
};

/** one state switch action */
class StateAction
{
  public:
    const std::string& getDeviceAction() const
    {
      return deviceAction;
    }

    void setDeviceAction(const std::string &deviceAction)
    {
      this->deviceAction = deviceAction;
    }

    uint32_t getDeviceId() const
    {
      return deviceId;
    }

    void setDeviceId(uint32_t deviceId)
    {
      this->deviceId = deviceId;
    }

    const Enum<StateMachineType>& getWhichStateMachine() const
    {
      return which;
    }

    void setStateMachine(const Enum<StateMachineType> &which)
    {
      this->which = which;
    }

  protected:
    uint32_t deviceId;
    Enum<StateMachineType> which { StateMachineType::Power };
    std::string deviceAction;
};

}
}
}
