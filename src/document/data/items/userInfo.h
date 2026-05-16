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

    const std::string& getFirstName() const
    {
      return firstName;
    }

    void setFirstName(const std::string &firstName)
    {
      this->firstName = firstName;
    }

    const std::string& getLastName() const
    {
      return lastName;
    }

    void setLastName(const std::string &lastName)
    {
      this->lastName = lastName;
    }

    const std::string& getFileCreationDate() const
    {
      return fileCreationDate;
    }

    void setFileCreationDate(const std::string &fileCreationDate)
    {
      this->fileCreationDate = fileCreationDate;
    }

    const std::string& getFileModificationDate() const
    {
      return fileModificationDate;
    }

    void setFileModificationDate(const std::string &fileModificationDate)
    {
      this->fileModificationDate = fileModificationDate;
    }

    const std::string& getOsUserName() const
    {
      return osUserName;
    }

    void setOsUserName(const std::string &osUserName)
    {
      this->osUserName = osUserName;
    }

    PROPERTY_GETTER(Property<bool>, newDeviceFound, getNewDeviceFound)
    PROPERTY_GETTER(Property<bool>, trainingWheels, getTrainingWheels)
    PROPERTY_GETTER(Property<Enum<Locale>>, locale, getLocale)
    PROPERTY_GETTER(Property<Enum<TimeFormat>>, timeFormat, getTimeFormat)

    const std::vector<UnknownElement> &getUnknownItems() const
    {
      return u;
    }

    void setUnknownItems(const std::vector<UnknownElement> &u)
    {
      this->u = u;
    }

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return u;
    }

    void setUnknownProperties(const std::vector<UnknownElement> &u)
    {
      p.u = u;
    }

  protected:
    struct Properties {
      Property<bool> newDeviceFound{false, Include::CHECK};
      Property<bool> trainingWheels{false, Include::ALWAYS};
      Property<Enum<Locale>> locale{{Locale::enu}, Include::ALWAYS};
      Property<Enum<TimeFormat>> timeFormat{{TimeFormat::Military}, Include::ALWAYS};
      std::vector<UnknownElement> u;
    } p;

    uint32_t id = DEFAULT_USERID;
    std::string firstName = "Michael";
    std::string lastName = "Mustermann";
    std::string osUserName;
    std::string fileCreationDate;
    std::string fileModificationDate;
    std::vector<UnknownElement> u;
};

}
}
}
