// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once


// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include "unknown.h"

namespace document
{
namespace data
{
namespace item
{

/** stores user info struct from UserConfiguration.xml
 */
class ControllerInfo
{
  public:
    static constexpr int DEFAULT_ID = 0;

    uint32_t getId() const
    {
      return id;
    }

    void setId(uint32_t id = DEFAULT_ID)
    {
      this->id = id;
    }

    PropertyString type{"Protocol Code"};
    PropertyString mnf{"Logitech"};
    PropertyString model{"Harmony 1000-ish"};
    PropertyString label{"Harmony 1000-ish"};

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return unknownProperties;
    }

    std::vector<UnknownElement> &getUnknownProperties()
    {
      return unknownProperties;
    }

  protected:
    uint32_t id = DEFAULT_ID;
    std::vector<UnknownElement> unknownProperties;
};

}
}
}


