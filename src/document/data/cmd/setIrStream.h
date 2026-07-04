// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/commands.h"

namespace document
{
namespace data
{

class SetIrStreamsCommand: public BaseCommand
{
    Q_OBJECT
  public:
    SetIrStreamsCommand(ConfigData &c, const binary::ssIr::File &file, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    ConfigData &c;
    binary::ssIr::File file;
    binary::ssIr::File prevFile;
};

class AddIrStreamtemCommand: public BaseCommand
{
    Q_OBJECT
  public:
    AddIrStreamtemCommand(ConfigData &c, const binary::TimingStream &stream, double clock, int pos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    int pos = -1;
    binary::TimingStream stream;
    double clock;
};




}
}
