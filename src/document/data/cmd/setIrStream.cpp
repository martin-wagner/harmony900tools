// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setIrStream.h"

using namespace std;

namespace document
{
namespace data
{

SetIrStreamsCommand::SetIrStreamsCommand(ConfigData &c,
    const binary::ssIr::File &file, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set IR Stream file"), parent), c(c), file(file)
{
  prevFile = c.getStreamLib();
}

void SetIrStreamsCommand::redo()
{
  c.getStreamLib() = file;
  emit dirtyChanged(true);
}

void SetIrStreamsCommand::undo()
{
  c.getStreamLib() = prevFile;
  emit dirtyChanged(true);
}

AddIrStreamtemCommand::AddIrStreamtemCommand(ConfigData &c,
    const binary::TimingStream &stream, double clock, int pos,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add IR Stream item"), parent), c(c), stream(
        stream), clock(clock)
{
  auto protCount = c.getStreamLib().getStreamCount();

  if (pos < 0) {
    //append
    pos = protCount;
  }
  if (pos > protCount) {
    return;
  }
  this->pos = pos;
  isValid = true;
}

void AddIrStreamtemCommand::redo()
{
  if (!isValid) {
    return;
  }

  c.getStreamLib().insertStream(stream, clock, pos);
  emit dirtyChanged(true);

}

void AddIrStreamtemCommand::undo()
{
  if (!isValid) {
    return;
  }

  c.getStreamLib().removeStream(pos);
  emit dirtyChanged(true);
}

bool AddIrStreamtemCommand::valid() const
{
  return isValid;
}

}
}
