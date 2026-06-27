// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveIrFromButtonCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveIrFromButtonCommand(const std::string &name, uint32_t devicePos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveIrProtoCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveIrProtoCommand(ConfigData &c, uint32_t devicePos, uint32_t cmdPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t cmdPos;
    item::ProtoCommand proto;
};

class RemoveIrRawCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveIrRawCommand(ConfigData &c, uint32_t devicePos, uint32_t cmdPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t cmdPos;
    item::RawCommand raw;
};


}
}
