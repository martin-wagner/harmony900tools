// SPDX-License-Identifier: LGPL-2.1-or-later

#include "catalogue.h"

#include "document/config.h"
#include "cmd/addDevice.h"
#include "cmd/removeDevice.h"

using namespace std;

namespace document
{
namespace data
{

CmdCatalogue::CmdCatalogue(Config &c, lib::UndoStack &undo, QObject *parent) :
    QObject(parent), c(c), undo(undo), uid(lib::UidGenerator::getInstance())
{
}

bool CmdCatalogue::addDeviceCommand(uint32_t *id)
{
  auto *cmd = new AddDeviceCommand(c);
  if (id != nullptr) {
    *id = cmd->getUid();
  }
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::removeDeviceCommand(uint32_t id)
{
  auto *cmd = new RemoveDeviceCommand(c, id);
  auto ret = cmd->valid();
  if (ret == true) {
    undo.push(cmd);
  } else {
    delete cmd;
  }
  return ret;
}

}
}

