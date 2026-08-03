// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveDeviceButtonCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceButtonCommand(ConfigData &c, uint32_t devicePos, item::ButtonType t, uint32_t buttonPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    item::ButtonType type;
    uint32_t devicePos;
    uint32_t buttonPos;
    item::Button button;
};

class RemoveActivityButtonCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveActivityButtonCommand(ConfigData &c, uint32_t activityPos, item::ButtonType t, uint32_t buttonPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    item::ButtonType type;
    uint32_t activityPos;
    uint32_t buttonPos;
    item::Button button;
};



}
}
