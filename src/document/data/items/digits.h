// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <array>

#include "action.h"
#include "document/data/property.h"

namespace document
{
namespace data
{
namespace item
{

/** links actions to numpad items
 *
 * index 0 ... 9 matches to numpad number 0 ... 9
 */
using Digits = std::array<DeviceAction, 10>;

enum class DigitSection {
  First,
  Middle,
  Last,
  Start,
  GreaterTen,
  GreaterHundred,
  Finish
};

/** class for storing how a number is sent by the remote. for more see the docs */
class Numpad {
  public:
    PropertyU32 fixedDigits{0, Used::NO};

    std::optional<DeviceAction> start;
    std::optional<DeviceAction> greaterTen;
    std::optional<DeviceAction> greaterHundred;
    std::optional<DeviceAction> finish;
    std::optional<Digits> first;
    std::optional<Digits> middle;
    std::optional<Digits> last;
};

}
}
}
