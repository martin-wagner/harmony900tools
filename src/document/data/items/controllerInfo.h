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

    const std::string& getLabel() const
    {
      return label;
    }

    void setLabel(const std::string &label)
    {
      this->label = label;
    }

    const std::string& getMnf() const
    {
      return mnf;
    }

    void setMnf(const std::string &mnf)
    {
      this->mnf = mnf;
    }

    const std::string& getModel() const
    {
      return model;
    }

    void setModel(const std::string &model)
    {
      this->model = model;
    }

    const std::string& getType() const
    {
      return type;
    }

    void setType(const std::string &type)
    {
      this->type = type;
    }

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
      return p.u;
    }

    void setUnknownProperties(const std::vector<UnknownElement> &u)
    {
      p.u = u;
    }

  protected:
    struct Properties {
      std::vector<UnknownElement> u;
    } p;

    uint32_t id = DEFAULT_ID;
    std::string type = "Protocol Code";
    std::string mnf = "Logitech";
    std::string model = "Harmony 1000-ish";
    std::string label = "Harmony 1000-ish";
    std::vector<UnknownElement> u;
};

}
}
}


