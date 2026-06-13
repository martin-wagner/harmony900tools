// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveButtonFromActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveButtonFromActivityCommand(const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveButtonCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveButtonCommand(ConfigData &c, item::ButtonType t, uint32_t devicePos, uint32_t buttonPos, QUndoCommand *parent = nullptr);

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

}
}
