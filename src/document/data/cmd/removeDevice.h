// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveDeviceFromActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceFromActivityCommand(std::set<uint32_t> ids, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveDeviceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceCommand(ConfigData &c, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t pos;
    item::Device device;
};

}
}
