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
    AddDeviceCommand(Config &c, QUndoCommand *parent = nullptr) :
        QUndoCommand(QObject::tr("Add device"), parent), c(c)
    {
      id = lib::UidGenerator::getInstance().generate();
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

  protected:
    Config &c;
    uint32_t id;
};

}
}

