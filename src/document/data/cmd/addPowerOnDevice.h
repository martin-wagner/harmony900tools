// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/commands.h"

namespace document
{
namespace data
{

class AddPowerOnDevice: public BaseCommand
{
    Q_OBJECT
  public:
    // devicePos -1 = append.
    AddPowerOnDevice(ConfigData &c, uint32_t activityPos, uint32_t id, int devicePos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    bool overwrite = false;

    ConfigData &c;
    uint32_t activityPos;
    int devicePos = -1;
    uint32_t id;
};

class AddPowerOffDevice: public BaseCommand
{
    Q_OBJECT
  public:
    // devicePos -1 = append.
    AddPowerOffDevice(ConfigData &c, uint32_t activityPos, uint32_t id, int devicePos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    bool overwrite = false;

    ConfigData &c;
    uint32_t activityPos;
    int devicePos = -1;
    uint32_t id;
};

}
}
