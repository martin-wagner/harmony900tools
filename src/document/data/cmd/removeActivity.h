// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveActivityCommand(ConfigData &c, uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t pos;
    item::Activity activity;
};

}
}
