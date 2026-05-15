// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>

#include "document/domain/enum.h"

namespace document
{
namespace domain
{
namespace item
{

/** stores which device is responsible for base functions.
 *
 * maybe logitech UI use only?
 */
class Role
{
  public:
    Role();
    Role(uint32_t deviceId, DeviceRole r) :
        deviceId(deviceId), role(Enum<DeviceRole>(r))
    {
    }

    Role(uint32_t deviceId, const std::string &r) :
        deviceId(deviceId), role(Enum<DeviceRole>(r))
    {
    }

    uint32_t getDeviceId() const
    {
      return deviceId;
    }

    void setDeviceId(uint32_t deviceId)
    {
      this->deviceId = deviceId;
    }

    const Enum<DeviceRole>& getRole() const
    {
      return role;
    }

    void setRole(const Enum<DeviceRole> &role)
    {
      this->role = role;
    }

  protected:
    uint32_t deviceId = 0;
    Enum<DeviceRole> role { DeviceRole::DEFAULT };
};

}
}
}
