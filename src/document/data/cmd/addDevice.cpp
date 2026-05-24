// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addDevice.h"

using namespace std;

namespace document
{
namespace data
{

AddDeviceCommand::AddDeviceCommand(ConfigData &c, QUndoCommand *parent) :
    BaseCommand("", parent), c(c)
{
  id = lib::UidGenerator::getInstance().generate();
  setText(QObject::tr("Add device (ID: %1)").arg(id));
  isValid = true;
}

AddDeviceCommand::AddDeviceCommand(ConfigData &c, uint32_t id,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add device (ID: %1)").arg(id), parent), c(c), id(
        id)
{
  auto &d = c.getDevices();
  if (!d.contains(id)) {
    isValid = true;
  }
}

void AddDeviceCommand::redo()
{
  item::Device device(id);
  c.getDevices().insert(make_pair(id, device));
  emit deviceAdded(id);
}

void AddDeviceCommand::undo()
{
  emit deviceAboutToBeRemoved(id);
  c.getDevices().erase(id);
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
