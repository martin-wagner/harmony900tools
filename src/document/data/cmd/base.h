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

class BaseCommand: public QObject, public QUndoCommand
{
  Q_OBJECT

  public:
    explicit BaseCommand(const QString &text, QUndoCommand *parent = nullptr);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    //...aboutToBe... must be closed using the corresponding function!!
    void deviceChanged(uint32_t pos);
    void deviceAboutToBeAdded(uint32_t pos);
    void deviceAdded(uint32_t pos);
    void deviceAboutToBeRemoved(uint32_t pos);
    void deviceRemoved(uint32_t pos);
    void activityChanged(uint32_t pos);
    void activityAboutToBeAdded(uint32_t pos);
    void activityAdded(uint32_t pos);
    void activityAboutToBeRemoved(uint32_t pos);
    void activityRemoved(uint32_t pos);
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

    void redo() override
    {
      set(value);
      emit dirtyChanged(true);
    }

    void undo() override
    {
      set(prevValue);
      emit dirtyChanged(true);
    }

  protected:
    T value;
    T prevValue;
    Getter get;
    Setter set;
};


}
}
