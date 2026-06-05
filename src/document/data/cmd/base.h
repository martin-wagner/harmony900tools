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

}
}
