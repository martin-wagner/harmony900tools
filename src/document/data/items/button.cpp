// SPDX-License-Identifier: LGPL-2.1-or-later

#include "button.h"

using namespace std;

namespace document
{
namespace data
{
namespace item
{

std::string Button::getAction(const std::string &aid)
{
  size_t first = aid.find('_');
  size_t last = aid.rfind('_');
  if (first == std::string::npos || first == last) {
      return "unknown";
  }
  return aid.substr(first + 1, last - first - 1);
}

}
}
}

