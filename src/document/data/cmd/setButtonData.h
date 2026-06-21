// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

inline item::Button& getButtonFromDeviceRef(ConfigData &c,
    item::ButtonType t, uint32_t devicePos, int buttonPos)
{
  if (t == item::ButtonType::Hard) {
    return c.getDevices()[devicePos].getHardButtons()[buttonPos];
  } else {
    return c.getDevices()[devicePos].getSoftButtons()[buttonPos];
  }
}

}
}
