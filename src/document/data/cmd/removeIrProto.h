// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveIrProtoLibItemFromIrProtoCommand: public BaseCommand
{
  Q_OBJECT
  public:
  RemoveIrProtoLibItemFromIrProtoCommand(uint32_t protocolIndex, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveIrProtoLibItemCommand: public BaseCommand
{
  Q_OBJECT
  public:
  RemoveIrProtoLibItemCommand(ConfigData &c, int pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    int pos = -1;
    binary::irProto::IrProto prot;
};


}
}
