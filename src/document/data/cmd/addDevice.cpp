// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>

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

  if (c.getDevices().size() + 1 > ConfigData::DEVICE_LIMIT) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, tr("Add device"),
        tr("According to the manual, you can have a max of %1 devices. "
            "By adding another one you will exceed this number. You've "
            "been warned...").arg(ConfigData::DEVICE_LIMIT),
        QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
    if (reply == QMessageBox::Abort) {
      return;
    }
  }

  setPos();
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
  setPos();
}

AddDeviceCommand::AddDeviceCommand(const item::Device &device, ConfigData &c,
    int pos, QUndoCommand *parent) :
    AddDeviceCommand(c, pos, parent)
{
  if (!isValid) {
    return;
  }

  this->device.emplace(device, id); //assing new id
}

void AddDeviceCommand::redo()
{
  if (!isValid) {
    return;
  }

  //todo newdevicefound property

  emit itemAboutToBeAdded(Item::DEVICE, pos);
  if (device.has_value()) {
    c.getDevices().insert(c.getDevices().begin() + pos, *device);
  } else {
    c.getDevices().insert(c.getDevices().begin() + pos, item::Device(id));
  }
  emit itemAdded(Item::DEVICE, pos);
  emit dirtyChanged(true);
}

void AddDeviceCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::DEVICE, pos);
  c.getDevices().erase(c.getDevices().begin() + pos);
  emit itemRemoved(Item::DEVICE, pos);
  emit dirtyChanged(true);
}

uint32_t AddDeviceCommand::getUid() const
{
  return id;
}

bool AddDeviceCommand::valid() const
{
  return isValid;
}

void AddDeviceCommand::setPos()
{
  if (pos < 0) {
    pos = c.getDevices().size();
  }
}

}
}
