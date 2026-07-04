// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

//todo remove command from command list (1:1 button/command. / check?)

class RemoveButtonFromActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveButtonFromActivityCommand(const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveDeviceButtonCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceButtonCommand(ConfigData &c, uint32_t devicePos, uint32_t buttonPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t buttonPos;
    item::Button button;
};

class RemoveActivityButtonCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveActivityButtonCommand(ConfigData &c, uint32_t activityPos, uint32_t buttonPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t activityPos;
    uint32_t buttonPos;
    item::Button button;
};



}
}
