// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setId.h"

using namespace std;

namespace document
{
namespace data
{

SetUserIdCommand::SetUserIdCommand(ConfigData &c, uint32_t id,
    QUndoCommand *parent) :
    SetIdCommand([&c]() -> uint32_t {
      return c.getUser().getId();
    }, [&c](uint32_t id) {
      c.getUser().setId(id);
    }, id, parent)
{
}

SetControllerIdCommand::SetControllerIdCommand(ConfigData &c, uint32_t id,
    QUndoCommand *parent) :
    SetIdCommand([&c]() -> uint32_t {
      return c.getController().getId();
    }, [&c](uint32_t id) {
      c.getController().setId(id);
    }, id, parent)
{
}

}
}
