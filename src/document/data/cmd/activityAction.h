// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/state.h"

namespace document
{
namespace data
{

class AddActivityActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddActivityActionCommand(ConfigData &c, uint32_t activityPos, item::ActivityAction t, int actionPos = -1,  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t activityPos;
    uint32_t actionPos;
    item::ActivityAction t;
};

class RemoveActivityActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveActivityActionCommand(ConfigData &c, uint32_t activityPos, item::ActivityAction t, uint32_t actionPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t activityPos;
    item::ActivityAction t;
    uint32_t actionPos;
    item::DeviceAction action;
};

class SetActivityActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetActivityActionCommand(ConfigData &c, uint32_t activityPos, item::ActivityAction t, int actionPos, const item::DeviceAction &act, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t activityPos;
    item::ActivityAction t;
    int actionPos;
    item::DeviceAction act;
    item::DeviceAction oldAct;
};



}
}
