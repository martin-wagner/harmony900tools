// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "document/data/enum.h"
#include "document/data/property.h"
#include "unknown.h"

namespace document
{
namespace data
{
namespace item
{

/** stores user info struct from UserConfiguration.xml
 */
class UserInfo
{
  public:
    //assigned by server -- not available for new projects...
    static constexpr int DEFAULT_USERID = 0x1ee7;

    uint32_t getId() const
    {
      return id;
    }

    void setId(uint32_t id = DEFAULT_USERID)
    {
      this->id = id;
    }

    PropertyString firstName{"Michael"};
    PropertyString lastName{"Mustermann"};
    PropertyString osUserName{""};
    PropertyString fileCreationDate{""};
    PropertyString fileModificationDate{""};

    PropertyBool newDeviceFound{false, Include::CHECK};
    PropertyBool trainingWheels{false, Include::ALWAYS};
    PropertyEnum<Locale> locale{{Locale::enu}, Include::ALWAYS};
    PropertyEnum<TimeFormat> timeFormat{{TimeFormat::Military}, Include::ALWAYS};

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return unknownProperties;
    }

    std::vector<UnknownElement> &getUnknownProperties()
    {
      return unknownProperties;
    }

  protected:
    uint32_t id = DEFAULT_USERID;
    std::vector<UnknownElement> unknownProperties;
};

}
}
}
