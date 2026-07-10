// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveRoleCommand: public BaseCommand
{
  Q_OBJECT
  public:
    static RemoveRoleCommand *fromId(ConfigData &c, uint32_t activityPos, uint32_t deviceId, QUndoCommand *parent = nullptr);
    RemoveRoleCommand(ConfigData &c, uint32_t activityPos, uint32_t rolePos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t activityPos;
    uint32_t rolePos;
    item::Role role;
};


}
}
