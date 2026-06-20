// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveActionFromActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveActionFromActivityCommand(const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};


class RemoveStateCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveStateCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateTransitionType t, uint32_t actPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateTransitionType t;
    uint32_t actPos;
    item::DeviceAction action;
    std::string name;
};

class RemoveActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //for relative state machine, actions are indipendent of states, therefore must be removed seperately
    RemoveActionCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t,  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    item::StateTransitionAction t;
    item::DeviceAction action;
};

//todo relative action remove state

}
}
