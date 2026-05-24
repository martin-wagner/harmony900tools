// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeDevice.h"

using namespace std;

namespace document
{
namespace data
{

RemoveDeviceFromActivityCommand::RemoveDeviceFromActivityCommand(
    set<uint32_t> ids, item::Activity &a, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove device from activity"), parent)
{
  // todo implement this!
}

void RemoveDeviceFromActivityCommand::redo()
{
}

void RemoveDeviceFromActivityCommand::undo()
{
}

RemoveDeviceCommand::RemoveDeviceCommand(ConfigData &c, uint32_t id,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove device (ID: %1)").arg(id), parent), c(c), id(
        id), device(0)
{
  auto &d = c.getDevices();
  if (d.contains(id)) {
    // copy device for undo
    device = d.at(id);

    // add activities for undo
    auto ids = d.at(id).getAllIds();
    for (auto &a : c.getActivities()) {
      auto *cmd = new RemoveDeviceFromActivityCommand(ids, a.second, this);

      connect(cmd, &RemoveDeviceFromActivityCommand::writeLog, this,
          &RemoveDeviceCommand::writeLog);
      connect(cmd, &RemoveDeviceFromActivityCommand::writeMsg, this,
          &RemoveDeviceCommand::writeMsg);
      connect(cmd, &RemoveDeviceFromActivityCommand::activityAdded, this,
          &RemoveDeviceCommand::activityAdded);
      connect(cmd, &RemoveDeviceFromActivityCommand::activityAboutToBeRemoved,
          this, &RemoveDeviceCommand::activityAboutToBeRemoved);
      connect(cmd, &RemoveDeviceFromActivityCommand::activityRemoved, this,
          &RemoveDeviceCommand::activityRemoved);
    }

    isValid = true;
  }
}

void RemoveDeviceCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit deviceAboutToBeRemoved(id);
  QUndoCommand::redo();
  c.getDevices().erase(id);
  emit deviceRemoved(id);
  emit dirtyChanged(true);
}

void RemoveDeviceCommand::undo()
{
  if (!isValid) {
    return;
  }

  c.getDevices().insert(make_pair(id, device));
  QUndoCommand::undo();
  emit deviceAdded(id);
  emit dirtyChanged(true);
}

bool RemoveDeviceCommand::valid() const
{
  return isValid;
}

}
}
