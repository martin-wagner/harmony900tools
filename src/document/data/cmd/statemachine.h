// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "document/data/enum.h"
#include "base.h"

namespace document
{
namespace data
{

class EditStateMachineCommand: public BaseCommand
{
  public:
    EditStateMachineCommand(ConfigData &c, uint32_t devicePos, const QString &desc, QUndoCommand *parent = nullptr);
    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;

  protected:
    bool checkRemove(const item::StateMachine &oldSm, const item::StateMachine &sm);
    bool checkRemove(StateMachineDeviceType type);
    bool checkDeviceAction(const item::DeviceAction &d, uint32_t deviceId, StateMachineDeviceType type);
};

class AddStatemachineCommand: public EditStateMachineCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    AddStatemachineCommand(ConfigData &c, uint32_t devicePos, int smPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    int smPos = -1;
};

class SetStatemachineCommand: public EditStateMachineCommand
{
  Q_OBJECT
  public:
    SetStatemachineCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, const item::StateMachine &sm, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    uint32_t smPos;
    item::StateMachine sm;
    item::StateMachine oldSm;
};

class RemoveStatemachineCommand: public EditStateMachineCommand
{
  Q_OBJECT
  public:
    RemoveStatemachineCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    uint32_t smPos;
    item::StateMachine sm;
};

}
}
