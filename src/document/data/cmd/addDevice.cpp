// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addDevice.h"

using namespace std;

namespace document
{
namespace data
{

AddDeviceCommand::AddDeviceCommand(ConfigData &c, int pos, QUndoCommand *parent) :
    BaseCommand("", parent), c(c), pos(pos)
{
  id = lib::UidGenerator::getInstance().generate();
  setText(QObject::tr("Add device (Id: %1, Pos: %2)").arg(id).arg(pos));
  isValid = true;
}

AddDeviceCommand::AddDeviceCommand(ConfigData &c, uint32_t id, int pos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add device (Id: %1, Pos: %2)").arg(id).arg(pos),
        parent), c(c), id(id), pos(pos)
{
  if (c.getDevice(id) == nullptr) {
    isValid = true;
  }
  if (pos > c.getDevices().size()) {
    //append
    pos = -1;
  }
}

void AddDeviceCommand::redo()
{
  if (!isValid) {
    return;
  }

  if (pos < 0) {
    c.getDevices().push_back(item::Device(id));
  } else {
    c.getDevices().insert(c.getDevices().begin() + pos, item::Device(id));
  }
  emit deviceAdded(id);
}

void AddDeviceCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit deviceAboutToBeRemoved(id);
  if (pos < 0) {
    c.getDevices().pop_back();
  } else {
    c.getDevices().erase(c.getDevices().begin() + pos);
  }
  emit deviceRemoved(id);
}

uint32_t AddDeviceCommand::getUid() const
{
  return id;
}

bool AddDeviceCommand::valid() const
{
  return isValid;
}

}
}
