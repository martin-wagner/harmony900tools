// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <set>
#include <QUndoCommand>

#include "lib/undo.h"
#include "lib/uid.h"
#include "document/data/data.h"
#include "ui/logViewer.h"

namespace document
{
namespace data
{

enum class Item
{
  UNKNOWN,
  USER,
  CONTROLLER,
  DEVICE,
  DEVICE_HARD_BUTTON,
  DEVICE_SOFT_BUTTON,
  DEVICE_STATEMACHINE,
  DEVICE_NUMPAD,
  DEVICE_IR,
  DEVICE_IR_DATA,
  ACTIVITY,
  ACTIVITY_CHANNEL,
  ACTIVITY_HARD_BUTTON,
  ACTIVITY_SOFT_BUTTON,
  ACTIVITY_ACTION,
  ACTIVITY_POWER,

  IR,
};

class BaseCommand: public QObject, public QUndoCommand
{
  Q_OBJECT

  public:
    explicit BaseCommand(const QString &text, QUndoCommand *parent = nullptr);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    //we hand over the info about item type and item position (if available, normally row).
    //We assume that only visible data can be changed, so parent info is not needed
    //The worst that can happen is the ui updates data that wasn't modified
    //...aboutToBe... must always be closed using the corresponding function!!
    void itemChanged(Item item, uint32_t pos);
    void itemAboutToBeAdded(Item item, uint32_t pos);
    void itemAdded(Item item, uint32_t pos);
    void itemAboutToBeRemoved(Item item, uint32_t pos);
    void itemRemoved(Item item, uint32_t pos);
    void dirtyChanged(bool dirty);
};

template<typename T>
class SetPropertyBaseCommand: public BaseCommand
{
  public:
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T&)>;

    SetPropertyBaseCommand(const QString &text, Getter getter, Setter setter,
        const T &value, QUndoCommand *parent = nullptr) :
        BaseCommand(text, parent), value(value), prevValue(getter()), get(
            getter), set(setter)
    {
    }

    void setChangedSignal(Item item, uint32_t pos)
    {
      changedItem = item;
      changedPos = pos;
    }

    void redo() override
    {
      set(value);
      if (changedItem != Item::UNKNOWN) {
        emit itemChanged(changedItem, changedPos);
      }
      emit dirtyChanged(true);
    }

    void undo() override
    {
      set(prevValue);
      if (changedItem != Item::UNKNOWN) {
        emit itemChanged(changedItem, changedPos);
      }
      emit dirtyChanged(true);
    }

    bool valid() const
    {
      return true;
    }

  protected:
    bool emitItemChangedSignal = false;
    uint32_t changedPos = 0;
    Item changedItem = Item::UNKNOWN;
    T value;
    T prevValue;
    Getter get;
    Setter set;
};

}
}
