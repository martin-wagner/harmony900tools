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

CmdCatalogue::CmdCatalogue(ConfigData &c, lib::UndoStack &undo, QObject *parent) :
    QObject(parent), c(c), undo(undo), uid(lib::UidGenerator::getInstance())
{
}

bool CmdCatalogue::addDeviceCommand(int pos, uint32_t id)
{
  auto *cmd = new AddDeviceCommand(c, id, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device pos %1 already exists, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::addDeviceCommand(int pos, uint32_t *id)
{
  auto *cmd = new AddDeviceCommand(c, pos);
  if (id != nullptr) {
    *id = cmd->getUid();
  }
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::removeDeviceCommand(int pos)
{
  auto *cmd = new RemoveDeviceCommand(c, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device pos %1 doesn't exist, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

void CmdCatalogue::connectCommand(BaseCommand *cmd)
{
  //connect signals. not all commands actually use all signals!

  // @formatter:off
  connect(cmd, &BaseCommand::writeLog, this, &CmdCatalogue::writeLog);
  connect(cmd, &BaseCommand::writeMsg, this, &CmdCatalogue::writeMsg);
  connect(cmd, &BaseCommand::deviceChanged, this, &CmdCatalogue::deviceChanged);
  connect(cmd, &BaseCommand::deviceAboutToBeAdded, this, &CmdCatalogue::deviceAboutToBeAdded);
  connect(cmd, &BaseCommand::deviceAdded, this, &CmdCatalogue::deviceAdded);
  connect(cmd, &BaseCommand::deviceAboutToBeRemoved, this, &CmdCatalogue::deviceAboutToBeRemoved);
  connect(cmd, &BaseCommand::deviceRemoved, this, &CmdCatalogue::deviceRemoved);
  connect(cmd, &BaseCommand::activityChanged, this, &CmdCatalogue::activityChanged);
  connect(cmd, &BaseCommand::activityAboutToBeAdded, this, &CmdCatalogue::activityAboutToBeAdded);
  connect(cmd, &BaseCommand::activityAdded, this, &CmdCatalogue::activityAdded);
  connect(cmd, &BaseCommand::activityAboutToBeRemoved, this, &CmdCatalogue::activityAboutToBeRemoved);
  connect(cmd, &BaseCommand::activityRemoved, this, &CmdCatalogue::activityRemoved);
  connect(cmd, &BaseCommand::dirtyChanged, this, &CmdCatalogue::dirtyChanged);
// @formatter:on
}

}
}

