// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/unknown.h"

namespace document
{
namespace data
{

class SetUnknownPropertyCommand: public BaseCommand
{
  protected:
    using UnknownElement = document::data::item::UnknownElement;

  public:
    using Getter = std::function<std::vector<UnknownElement>&()>;
    using Setter = std::function<void(const UnknownElement&)>;

    SetUnknownPropertyCommand(Getter getter, Setter setter,
        const UnknownElement &value, QUndoCommand *parent = nullptr) :
        BaseCommand(
            QObject::tr("Unknown property (%1)").arg(
                QString::fromStdString(value.tag)), parent), value(value), get(
            getter), set(setter)
    {
    }

    void redo() override
    {
      set(value);
      emit dirtyChanged(true);
    }

    void undo() override
    {
      auto &vec = get();
      if (!vec.empty()) {
        vec.pop_back();
      }

      emit dirtyChanged(true);
    }

  private:
    UnknownElement value;
    Getter get;
    Setter set;
};

class SetUserUnknownPropertyCommand: public SetUnknownPropertyCommand
{
  public:
    SetUserUnknownPropertyCommand(ConfigData& c,
        const UnknownElement& value,
        QUndoCommand* parent = nullptr);
};

class SetControllerUnknownPropertyCommand: public SetUnknownPropertyCommand
{
  public:
    SetControllerUnknownPropertyCommand(ConfigData& c,
        const UnknownElement& value,
        QUndoCommand* parent = nullptr);
};

class SetDeviceUnknownPropertyCommand: public SetUnknownPropertyCommand
{
  public:
    SetDeviceUnknownPropertyCommand(ConfigData& c,
        const UnknownElement& value, uint32_t pos,
        QUndoCommand* parent = nullptr);
};

class SetDeviceStateActionUnknownParamCommand: public SetUnknownPropertyCommand
{
  public:
    SetDeviceStateActionUnknownParamCommand(ConfigData& c,
        const UnknownElement& value, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
        item::StateTransitionAction t, uint32_t seqPos,
        QUndoCommand* parent = nullptr);
};

class SetDeviceNumpadActionUnknownParamCommand: public SetUnknownPropertyCommand
{
  public:
    SetDeviceNumpadActionUnknownParamCommand(ConfigData &c,
        const UnknownElement &value, uint32_t devicePos, item::DigitSection s,
        uint32_t digit, uint32_t seqPos, QUndoCommand *parent = nullptr);
};

class SetIrUnknownPropertyCommand: public SetUnknownPropertyCommand
{
  public:
    SetIrUnknownPropertyCommand(ConfigData &c,
        const UnknownElement &value, uint32_t devicePos, QUndoCommand *parent = nullptr);
};

class SetActivityUnknownPropertyCommand: public SetUnknownPropertyCommand
{
  public:
    SetActivityUnknownPropertyCommand(ConfigData& c,
        const UnknownElement& value, uint32_t pos,
        QUndoCommand* parent = nullptr);
};



}
}
