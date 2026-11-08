// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class AddActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    AddActivityCommand(ConfigData &c, int pos = -1, QUndoCommand *parent = nullptr);
    //pos -1 = append
    AddActivityCommand(ConfigData &c, uint32_t id, int pos = -1, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    uint32_t getUid() const;
    bool valid() const;

    void setPos();

  protected:
    bool isValid = false;
    ConfigData &c;
    uint32_t id;
    int pos;
};


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

class RemoveActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveActivityCommand(ConfigData &c, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t pos;
    item::Activity activity;
};


}
}
