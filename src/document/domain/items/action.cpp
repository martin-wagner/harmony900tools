// SPDX-License-Identifier: LGPL-2.1-or-later

#include "action.h"

using namespace std;

namespace document
{
namespace domain
{
namespace item
{

DeviceAction::DeviceAction(uint32_t id, const std::string &cmd, bool canRepeat) :
    id(id), cmd(cmd), repeatWillNotHarm(canRepeat)
{
}

}
}
}
