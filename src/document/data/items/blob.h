// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <array>

#include "document/data/enum.h"

namespace document
{
namespace data
{
namespace item
{

using Permissions = std::array<uint8_t, 3>;

/** stores raw files
 */
class Blob
{
  public:
    Blob(const std::string &file, const std::vector<uint8_t> &data, const Permissions &p = {6,6,6}) :
      file(file), p(p), data(data)
    {
    }

  protected:
    std::string file;
    Permissions p;
    std::vector<uint8_t> data;
};

}
}
}



