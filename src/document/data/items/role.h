// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>

#include "document/data/enum.h"

namespace document
{
namespace data
{
namespace item
{

/** stores which device is responsible for base functions
 *
 * must be unique for any activity!
 */
class Role
{
  public:
    PropertyU32 deviceId{0};
    Enum<DeviceRole> role { DeviceRole::DEFAULT };
};

}
}
}
