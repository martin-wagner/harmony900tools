// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveStatemachineFromActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveStatemachineFromActivityCommand(const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveIrCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveIrCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateMachine sm;
};

}
}
