// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/commands.h"

namespace document
{
namespace data
{

class SetIrProtoLibCommand: public BaseCommand
{
    Q_OBJECT
  public:
    SetIrProtoLibCommand(ConfigData &c, const binary::irProto::File &file, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    ConfigData &c;
    binary::irProto::File file;
    binary::irProto::File prevFile;
};

class AddIrProtoLibItemCommand: public BaseCommand
{
    Q_OBJECT
  public:
    AddIrProtoLibItemCommand(ConfigData &c, const binary::irProto::IrProto &prot, int pos = -1, QUndoCommand *parent = nullptr);

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
