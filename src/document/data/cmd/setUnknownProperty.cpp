// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setUnknownProperty.h"
#include "setActionData.h"

using namespace std;

namespace document
{
namespace data
{

SetUserUnknownPropertyCommand::SetUserUnknownPropertyCommand(ConfigData &c,
    const UnknownElement &value, QUndoCommand *parent) :
    SetUnknownPropertyCommand([&c]() -> std::vector<UnknownElement>& {
      return c.getUser().getUnknownProperties();
    }, [&c](const UnknownElement &e) {
      c.getUser().getUnknownProperties().push_back(e);
    }, value, parent)
{
}

SetControllerUnknownPropertyCommand::SetControllerUnknownPropertyCommand(
    ConfigData &c, const UnknownElement &value, QUndoCommand *parent) :
    SetUnknownPropertyCommand([&c]() -> std::vector<UnknownElement>& {
      return c.getController().getUnknownProperties();
    }, [&c](const UnknownElement &e) {
      c.getController().getUnknownProperties().push_back(e);
    }, value, parent)
{
}

SetDeviceUnknownPropertyCommand::SetDeviceUnknownPropertyCommand(ConfigData &c,
    const UnknownElement &value, uint32_t pos, QUndoCommand *parent) :
    SetUnknownPropertyCommand([&c, pos]() -> std::vector<UnknownElement>& {
      return c.getDevices()[pos].getUnknownProperties();
    }, [&c, pos](const UnknownElement &e) {
      c.getDevices()[pos].getUnknownProperties().push_back(e);
    }, value, parent)
{
}

SetDeviceStateActionUnknownParamCommand::SetDeviceStateActionUnknownParamCommand(
    ConfigData &c, const UnknownElement &value, uint32_t devicePos,
    uint32_t smPos, uint32_t actPos, item::StateTransitionAction t,
    uint32_t seqPos, QUndoCommand *parent) :
    SetUnknownPropertyCommand([&c, devicePos, smPos, actPos, t, seqPos]() -> std::vector<UnknownElement>& {
      return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].getUnknownParams();
    }, [&c, devicePos, smPos, actPos, t, seqPos](const UnknownElement &e) {
      getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence[seqPos].getUnknownParams().push_back(e);
    }, value, parent)
{
}

}
}
