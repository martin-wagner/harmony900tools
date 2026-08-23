// SPDX-License-Identifier: LGPL-2.1-or-later

#include "base.h"

namespace document
{
namespace data
{

BaseCommand::BaseCommand(const QString &text, QUndoCommand *parent) :
    QObject(nullptr),
    QUndoCommand(text, parent)
{
}

void BaseCommand::connectCommand(BaseCommand *cmd)
{
  //connect signals. not all commands actually use all signals!

  // @formatter:off
  connect(this, &BaseCommand::writeLog, cmd, &BaseCommand::writeLog);
  connect(this, &BaseCommand::writeMsg, cmd, &BaseCommand::writeMsg);
  connect(this, &BaseCommand::itemChanged, cmd, &BaseCommand::itemChanged);
  connect(this, &BaseCommand::itemAboutToBeAdded, cmd, &BaseCommand::itemAboutToBeAdded);
  connect(this, &BaseCommand::itemAdded, cmd, &BaseCommand::itemAdded);
  connect(this, &BaseCommand::itemAboutToBeRemoved, cmd, &BaseCommand::itemAboutToBeRemoved);
  connect(this, &BaseCommand::itemRemoved, cmd, &BaseCommand::itemRemoved);
  connect(this, &BaseCommand::dirtyChanged, cmd, &BaseCommand::dirtyChanged);
// @formatter:on

}

}
}
