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

class RemoveDeviceFromActivityCommand: public QUndoCommand
{
  public:
    RemoveDeviceFromActivityCommand(std::set<uint32_t> ids, item::Activity &a,
        QUndoCommand *parent = nullptr) :
        QUndoCommand(QObject::tr("Remove device from activity"), parent)
    {
      //todo implement this!
    }

    void redo() override
    {
    }

    void undo() override
    {
    }

  protected:

};

class RemoveDeviceCommand: public QUndoCommand
{
  public:
    RemoveDeviceCommand(Config &c, uint32_t id, QUndoCommand *parent = nullptr) :
        QUndoCommand(QObject::tr("Remove device"), parent), c(c), id(id), device(0)
    {
      auto &d = c.getDevices();
      if (d.contains(id)) {
        //copy device for undo

        //add activites for undo
        auto ids = d.at(id).getAllIds();
        for (auto &a : c.getActivities()) {
          new RemoveDeviceFromActivityCommand(ids, a.second, this);
        }

        isValid = true;
      }
    }

    void redo() override
    {
      if (!isValid) {
        return;
      }

      QUndoCommand::redo();
      c.getDevices().erase(id);
    }

    void undo() override
    {
      if (!isValid) {
        return;
      }
      c.getDevices().insert(std::make_pair(id, device));
      QUndoCommand::undo();
    }

    bool valid() const
    {
      return isValid;
    }

  protected:
    bool isValid = false;

    Config &c;
    uint32_t id;
    item::Device device;

};


}
}

