// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class EditStateCommand: public BaseCommand
{
  public:
    EditStateCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateMachineType t, const QString &desc, QUndoCommand *parent = nullptr);

    bool valid() const;

  public:
    static bool checkRemove(ConfigData &c, uint32_t devicePos, StateMachineDeviceType type, const QString &state);

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateMachineType t;

    bool checkRemove(StateMachineDeviceType type, const QString &state);
    static bool checkDeviceAction(const item::DeviceAction &a, uint32_t deviceId, StateMachineDeviceType type, const QString &state);
};

class AddStateCommand: public EditStateCommand
{
  Q_OBJECT
  public:
    AddStateCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, const QString &name, item::StateMachineType t, int actPos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    int actPos;
    std::string name;
};

class SetStateNameCommand: public EditStateCommand
{
  Q_OBJECT
  public:
    SetStateNameCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateMachineType t, uint32_t statePos, const QString &name, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    uint32_t statePos;
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


class RemoveStateCommand: public EditStateCommand
{
  Q_OBJECT
  public:
    RemoveStateCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateMachineType t, uint32_t actPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
  protected:
    uint32_t actPos;
    item::DeviceAction action;
    std::string name;
};

class RemoveActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //for relative state machine, actions are indipendent of states, therefore must be removed seperately
    RemoveActionCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t,  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateMachineAction t;
    item::DeviceAction action;
};

}
}
