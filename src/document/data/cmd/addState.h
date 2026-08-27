// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/state.h"

namespace document
{
namespace data
{

class AddStateCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddStateCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, const QString &name, item::StateMachineType t, int actPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateMachineType t;
    int actPos;
    std::string name;
};

class SetStateNameCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetStateNameCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateMachineType t, uint32_t statePos, const QString &name, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    uint32_t statePos;
    item::StateMachineType t;
    std::string name;
    std::string oldName;
};

class SetStateDeviceActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetStateDeviceActionCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, const item::DeviceAction &act, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    uint32_t actPos;
    item::StateMachineAction t;
    item::DeviceAction act;
    item::DeviceAction oldAct;
};

class AddActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //for state machine actions "start", "finish" that are applicable for all states
    //for relative state machine, actions are indipendent of states, therefore must be added seperately
    AddActionCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateMachineAction t;
};

}
}
