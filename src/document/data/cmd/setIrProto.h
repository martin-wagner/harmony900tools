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

class AppendIrProtoLibItemCommand: public BaseCommand
{
    Q_OBJECT
  public:
    AppendIrProtoLibItemCommand(ConfigData &c, const Enum<CodeType> &t, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

    int index() const;

  protected:
    bool isValid = false;

    binary::irProto::IrProto protocol;
    ConfigData &c;
    Enum<CodeType> t;
    int exists;
    int pos;
};




}
}
