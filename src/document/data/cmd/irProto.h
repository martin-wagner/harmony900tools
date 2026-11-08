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

class AppendLocalIrProtoLibItemCommand: public BaseCommand
{
    Q_OBJECT
  public:
    AppendLocalIrProtoLibItemCommand(ConfigData &c, const Enum<CodeType> &t, QUndoCommand *parent = nullptr);

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

class AppendDatabaseIrProtoLibItemCommand: public BaseCommand
{
    Q_OBJECT
  public:
    AppendDatabaseIrProtoLibItemCommand(ConfigData &c, const std::string &t, const binary::irProto::IrProto &proto, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

    int index() const;

  protected:
    bool isValid = false;

    binary::irProto::IrProto protocol;
    ConfigData &c;
    std::string t;
    int exists;
    int pos;
};




}
}
