// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class AddDeviceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    //pos -1 = append
    AddDeviceCommand(ConfigData &c, int pos = -1, QUndoCommand *parent = nullptr);
    //pos -1 = append
    AddDeviceCommand(ConfigData &c, uint32_t id, int pos = -1, QUndoCommand *parent = nullptr);
    //pos -1 = append
    AddDeviceCommand(const item::Device &device, ConfigData &c, int pos = -1, QUndoCommand *parent = nullptr);

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
    std::optional<item::Device> device;
};

}
}
