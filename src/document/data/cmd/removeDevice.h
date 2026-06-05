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
    RemoveDeviceFromActivityCommand(std::set<uint32_t> ids, item::Activity &a, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
};

class RemoveDeviceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = end
    RemoveDeviceCommand(ConfigData &c, int pos = -1, QUndoCommand *parent = nullptr);

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
