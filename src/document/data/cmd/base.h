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

    void deviceChanged(uint32_t id);
    void deviceAdded(uint32_t id);
    void deviceAboutToBeRemoved(uint32_t id);
    void deviceRemoved(uint32_t id);
    void activityChanged(uint32_t id);
    void activityAdded(uint32_t id);
    void activityAboutToBeRemoved(uint32_t id);
    void activityRemoved(uint32_t id);
    void dirtyChanged(bool dirty);
};

}
}
