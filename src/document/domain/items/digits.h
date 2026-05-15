// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <array>

#include "action.h"

namespace document
{
namespace domain
{
namespace item
{

/** links actions to numpad items
 *
 * index 0 ... 9 matches to numpad number 0 ... 9
 */
class Digits
{
  public:
    Digits()
    {
    }
    Digits(const std::array<DeviceAction, 10> &digits)
    {
      this->digits = digits;
    }

    const std::array<DeviceAction, 10>& getDigits() const
    {
      return digits;
    }

    void setDigits(const std::array<DeviceAction, 10> &digits)
    {
      this->digits = digits;
    }

    void setDigit(const DeviceAction &action, int index)
    {
      if (index < digits.size()) {
        this->digits = digits;
      }
    }

  protected:
    std::array<DeviceAction, 10> digits;
};

}
}
}
