// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class MoveActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    MoveActivityCommand(ConfigData &c, uint32_t currentPos, uint32_t newPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    void work(uint32_t posA, uint32_t posB);
    void remove(uint32_t pos);
    void add(item::Activity &act, uint32_t pos);

    bool isValid = false;
    ConfigData &c;
    uint32_t oldPos;
    uint32_t newPos;

};

}
}
