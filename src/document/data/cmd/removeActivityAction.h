// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{


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

}
}
