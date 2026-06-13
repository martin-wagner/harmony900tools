// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveDeviceActionFromActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceActionFromActivityCommand(const std::set<std::string> &actionIds, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveDeviceActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceActionCommand(ConfigData &c, uint32_t devicePos, uint32_t smPos, uint32_t actPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t smPos;
    uint32_t actPos;
    item::DeviceAction action;
};

}
}
