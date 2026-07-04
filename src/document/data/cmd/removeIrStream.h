// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveIrStreamItemFromIrStreamCommand: public BaseCommand
{
  Q_OBJECT
  public:
  RemoveIrStreamItemFromIrStreamCommand(uint32_t streamIndex, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveIrStreamtemCommand: public BaseCommand
{
  Q_OBJECT
  public:
  RemoveIrStreamtemCommand(ConfigData &c, int pos, QUndoCommand *parent = nullptr);

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
