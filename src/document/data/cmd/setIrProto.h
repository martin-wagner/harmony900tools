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

class SetIrProtoLibItemCommand: public BaseCommand
{
    Q_OBJECT
  public:
    SetIrProtoLibItemCommand(ConfigData &c, const binary::irProto::IrProto &prot, int pos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    bool overwrite = false;

    ConfigData &c;
    int pos = -1;
    binary::irProto::IrProto prot;
    binary::irProto::IrProto prevProt;
};




}
}
