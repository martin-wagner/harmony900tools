// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "unknownProperty.h"
#include "getActionData.h"

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
