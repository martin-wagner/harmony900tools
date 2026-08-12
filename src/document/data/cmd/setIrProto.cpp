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

AppendIrProtoLibItemCommand::AppendIrProtoLibItemCommand(ConfigData &c,
    const Enum<CodeType> &t, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Append IR Proto Lib item"), parent), c(c), t(t)
{
  exists = c.getPrococolLibIndex(t.getValue());
  if (exists >= 0) {
    //already exists. don't do anything
    pos = exists;
    isValid = true;
    return;
  }

  protocol = c.getProtocolCatalogue().get(t.getQString());
  if (protocol.isEmpty()) {
    return;
  }
  pos = c.getProtocolLib().getProtocolCount();
  isValid = true;
}

void AppendIrProtoLibItemCommand::redo()
{
  if (!isValid || (exists >= 0)) {
    return;
  }

  emit itemAboutToBeAdded(Item::IR_PROTO_LIB, pos);
  c.getProtocolLib().insertProtocol(protocol, pos);
  c.addProtocolLibListItem(t.getValue(), pos);
  emit itemAdded(Item::IR_PROTO_LIB, pos);
  emit dirtyChanged(true);
}

void AppendIrProtoLibItemCommand::undo()
{
  if (!isValid || (exists >= 0)) {
    return;
  }

  emit itemAboutToBeRemoved(Item::IR_PROTO_LIB, pos);
  c.removePrococolLibListItem(t.getValue());
  c.getProtocolLib().removeProtocol(pos);
  emit itemRemoved(Item::IR_PROTO_LIB, pos);
  emit dirtyChanged(true);
}

bool AppendIrProtoLibItemCommand::valid() const
{
  return isValid;
}

int AppendIrProtoLibItemCommand::index() const
{
  return pos;
}

}
}
