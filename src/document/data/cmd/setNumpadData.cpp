// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setNumpadData.h"

using namespace std;

namespace document
{
namespace data
{

SetNumpadFixedDigitsCommand::SetNumpadFixedDigitsCommand(ConfigData &c, uint32_t value,
    uint32_t devicePos, QUndoCommand *parent) :
    SetPropertyBaseCommand<uint32_t>(QObject::tr("set numpad fixed digits"),
        [&c, devicePos]() {
          return c.getDevices()[devicePos].getNumpad()->fixedDigits.get();
        },
        [&c, devicePos](const uint32_t v) {
          c.getDevices()[devicePos].getNumpad()->fixedDigits.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}


}
}
