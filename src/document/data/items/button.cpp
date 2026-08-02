// SPDX-License-Identifier: LGPL-2.1-or-later

#include "button.h"

using namespace std;

namespace document
{
namespace data
{
namespace item
{

string Button::getAction(const string &aid)
{
  size_t first = aid.find('_');
  size_t last = aid.rfind('_');
  if (first == string::npos || first == last) {
    return "unknown";
  }
  return aid.substr(first + 1, last - first - 1);
}

uint32_t Button::getDevice(const string &aid)
{
  size_t first = aid.find('_');
  if (first == string::npos) {
    return 0;
  }
  return stoul(aid.substr(0, first));
}

}
}
}

