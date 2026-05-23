// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <set>
#include <QUndoCommand>

#include "lib/undo.h"
#include "lib/uid.h"
#include "document/data/data.h"

namespace document
{
namespace data
{

class AddDeviceCommand: public QUndoCommand
{
  public:
    AddDeviceCommand(ConfigData &c, QUndoCommand *parent = nullptr) :
        QUndoCommand(parent), c(c)
    {
      id = lib::UidGenerator::getInstance().generate();
      setText(QObject::tr("Add device (ID: %1)").arg(id));
      isValid = true;
    }

    AddDeviceCommand(ConfigData &c, uint32_t id, QUndoCommand *parent = nullptr) :
        QUndoCommand(QObject::tr("Add device (ID: %1)").arg(id), parent), c(c), id(
            id)
    {
      auto &d = c.getDevices();
      if (!d.contains(id)) {
        isValid = true;
      }
    }

    void redo() override
    {
      item::Device device(id);
      c.getDevices().insert(std::make_pair(id, device));
    }

    void undo() override
    {
      c.getDevices().erase(id);
    }

    uint32_t getUid() const
    {
      return id;
    }

    bool valid() const
    {
      return isValid;
    }

  protected:
    bool isValid = false;
    ConfigData &c;
    uint32_t id;
};

}
}

