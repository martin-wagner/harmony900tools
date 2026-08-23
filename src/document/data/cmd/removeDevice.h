// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

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

  protected:
    bool checkRemove();

  private:
    bool checkActivityAction(const item::DeviceAction &d, uint32_t deviceId) const;
    bool doRemove(uint32_t deviceId);
};

}
}
