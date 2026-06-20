// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setActionData.h"

using namespace std;

namespace document
{
namespace data
{

SetActionTypeCommand::SetActionTypeCommand(ConfigData &c,
    const Enum<ActionType> &value, uint32_t devicePos, uint32_t smPos,
    item::StateTransitionAction t, uint32_t actPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<ActionType>>(QObject::tr("set device action type"),
        [&c, devicePos, smPos, t, actPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->actionType.get();
        },
        [&c, devicePos, smPos, t, actPos](const Enum<ActionType> &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->actionType.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetActionRepeatWillNotHarmCommand::SetActionRepeatWillNotHarmCommand(
    ConfigData &c, bool value, uint32_t devicePos, uint32_t smPos,
    item::StateTransitionAction t, uint32_t actPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set device action repeat will not harm"),
        [&c, devicePos, smPos, t, actPos]() {
          return getActionRef(c, devicePos, smPos, t, actPos)->repeatWillNotHarm.get();
        },
        [&c, devicePos, smPos, t, actPos](const bool &v) {
          getActionRef(c, devicePos, smPos, t, actPos)->repeatWillNotHarm.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

}
}
