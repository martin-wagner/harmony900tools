// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/unknown.h"

namespace document
{
namespace data
{

class SetIdCommand: public BaseCommand
{
  protected:
    using UnknownElement = document::data::item::UnknownElement;

  public:
    using Getter = std::function<uint32_t()>;
    using Setter = std::function<void(uint32_t)>;

    SetIdCommand(Getter getter, Setter setter, uint32_t id,
        QUndoCommand *parent = nullptr) :
        BaseCommand(QObject::tr("Set ID %1").arg(id), parent), id(
            id), prevId(getter()), get(getter), set(setter)
    {
    }

    void redo() override
    {
      set(id);
      emit dirtyChanged(true);
    }

    void undo() override
    {
      set(prevId);
      emit dirtyChanged(true);
    }

  private:
    uint32_t id;
    uint32_t prevId;
    Getter get;
    Setter set;
};

class SetUserIdCommand: public SetIdCommand
{
  public:
    SetUserIdCommand(ConfigData &c, uint32_t id,
        QUndoCommand *parent = nullptr);
};

class SetControllerIdCommand: public SetIdCommand
{
  public:
    SetControllerIdCommand(ConfigData &c, uint32_t id,
        QUndoCommand *parent = nullptr);
};

}
}
