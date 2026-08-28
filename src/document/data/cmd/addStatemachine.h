// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/state.h"

namespace document
{
namespace data
{

class AddStatemachineCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    AddStatemachineCommand(ConfigData &c, uint32_t devicePos, int smPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    int smPos = -1;
};

class SetStatemachineCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetStatemachineCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, const item::StateMachine &sm, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateMachine sm;
    item::StateMachine oldSm;
};

}
}
