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

SetActivityActionUnknownParamCommand::SetActivityActionUnknownParamCommand(
    ConfigData &c, const data::item::UnknownElement &value, uint32_t activityPos,
    item::ActivityAction t, int actionPos, uint32_t seqPos,
    QUndoCommand *parent) :
        SetUnknownPropertyCommand([&c, activityPos, t, actionPos, seqPos]() -> std::vector<UnknownElement>& {
          return getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].getUnknownParams();
        }, [&c, activityPos, t, actionPos, seqPos](const UnknownElement &e) {
          getActionFromActivity(c, activityPos, t, actionPos)->sequence[seqPos].getUnknownParams().push_back(e);
        }, value, parent)
{
}

SetDeviceNumpadActionUnknownParamCommand::SetDeviceNumpadActionUnknownParamCommand(
    ConfigData &c, const UnknownElement &value, uint32_t devicePos,
    item::DigitSection s, uint32_t digit, uint32_t seqPos, QUndoCommand *parent) :
    SetUnknownPropertyCommand(
        [&c, devicePos, s, digit, seqPos]() -> std::vector<UnknownElement>& {
          return getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].getUnknownParams();
        }, [&c, devicePos, s, digit, seqPos](const UnknownElement &e) {
          getActionFromNumpadRef(c, devicePos, s, digit)->sequence[seqPos].getUnknownParams().push_back(e);
        }, value, parent)
{
}

SetIrUnknownPropertyCommand::SetIrUnknownPropertyCommand(
    ConfigData &c, const UnknownElement &value, uint32_t devicePos, QUndoCommand *parent) :
        SetUnknownPropertyCommand([&c, devicePos]() -> std::vector<UnknownElement>& {
          return c.getDevices()[devicePos].getIrCommands().getUnknownProperties();
        }, [&c, devicePos](const UnknownElement &e) {
          c.getDevices()[devicePos].getIrCommands().getUnknownProperties().push_back(e);
        }, value, parent)
{
}

SetActivityUnknownPropertyCommand::SetActivityUnknownPropertyCommand(ConfigData &c,
    const UnknownElement &value, uint32_t pos, QUndoCommand *parent) :
    SetUnknownPropertyCommand([&c, pos]() -> std::vector<UnknownElement>& {
      return c.getActivities()[pos].getUnknownProperties();
    }, [&c, pos](const UnknownElement &e) {
      c.getActivities()[pos].getUnknownProperties().push_back(e);
    }, value, parent)
{
}

}
}
