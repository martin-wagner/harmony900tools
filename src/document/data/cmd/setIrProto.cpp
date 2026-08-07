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

SetIrProtoLibItemCommand::SetIrProtoLibItemCommand(ConfigData &c,
    const binary::irProto::IrProto &prot, int pos, bool overwrite,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add IR Proto Lib item"), parent), overwrite(
        overwrite), c(c), prot(prot)
{
  auto &protocols = c.getProtocolLib();
  auto protCount = protocols.getProtocolCount();

  if (overwrite) {
    if ((pos < 0) || (static_cast<uint32_t>(pos) >= protCount)) {
      return;
    }
    prevProt = protocols.accessProtocol(pos);
  } else {
    if (pos < 0) {
      //append
      pos = protCount;
    }
    if (pos > protCount) {
      return;
    }
  }

  this->pos = pos;
  isValid = true;
}

void SetIrProtoLibItemCommand::redo()
{
  if (!isValid) {
    return;
  }

  if (!overwrite) {
    emit itemAboutToBeAdded(Item::IR_PROTO_LIB, pos);
  }

  if (overwrite) {
    c.getProtocolLib().removeProtocol(pos);
  }
  c.getProtocolLib().insertProtocol(prot, pos);

  if (overwrite) {
    emit itemChanged(Item::IR_PROTO_LIB, pos);
  } else {
    emit itemAdded(Item::IR_PROTO_LIB, pos);
  }
  emit dirtyChanged(true);
}

void SetIrProtoLibItemCommand::undo()
{
  if (!isValid) {
    return;
  }

  if (!overwrite) {
    emit itemAboutToBeRemoved(Item::IR_PROTO_LIB, pos);
  }

  c.getProtocolLib().removeProtocol(pos);
  if (overwrite) {
    c.getProtocolLib().insertProtocol(prevProt, pos);
  }

  if (overwrite) {
    emit itemChanged(Item::IR_PROTO_LIB, pos);
  } else {
    emit itemRemoved(Item::IR_PROTO_LIB, pos);
  }
  emit dirtyChanged(true);
}

bool SetIrProtoLibItemCommand::valid() const
{
  return isValid;
}

}
}
