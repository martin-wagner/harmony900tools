// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setIrProto.h"

using namespace std;

namespace document
{
namespace data
{

SetIrProtoLibCommand::SetIrProtoLibCommand(ConfigData &c,
    const binary::irProto::File &file, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set IR Proto Lib file"), parent), c(c), file(file)
{
  prevFile = c.getProtocolLib();
}

void SetIrProtoLibCommand::redo()
{
  c.getProtocolLib() = file;
  emit dirtyChanged(true);
}

void SetIrProtoLibCommand::undo()
{
  c.getProtocolLib() = prevFile;
  emit dirtyChanged(true);
}

AddIrProtoLibItemCommand::AddIrProtoLibItemCommand(ConfigData &c,
    const binary::irProto::IrProto &prot, int pos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add IR Proto Lib item"), parent), c(c), prot(prot)
{
  auto protCount = c.getProtocolLib().getProtocolCount();

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

void AddIrProtoLibItemCommand::redo()
{
  if (!isValid) {
    return;
  }

  c.getProtocolLib().insertProtocol(prot, pos);
  emit dirtyChanged(true);

}

void AddIrProtoLibItemCommand::undo()
{
  if (!isValid) {
    return;
  }

  c.getProtocolLib().removeProtocol(pos);
  emit dirtyChanged(true);
}

bool AddIrProtoLibItemCommand::valid() const
{
  return isValid;
}

}
}
