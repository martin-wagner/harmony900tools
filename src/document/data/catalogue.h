// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "lib/undo.h"
#include "lib/uid.h"
#include "ui/logViewer.h"
#include "cmd/base.h"

namespace document
{
namespace data
{

class ConfigData;

/** all data modification commands, implementing dependencies and undo */
class CmdCatalogue : public QObject
{
  Q_OBJECT
  public:
    CmdCatalogue(ConfigData &c, lib::UndoStack &undo, QObject *parent = nullptr);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    void deviceChanged(uint32_t pos);
    void deviceAdded(uint32_t pos);
    void deviceAboutToBeRemoved(uint32_t pos);
    void deviceRemoved(uint32_t pos);
    void activityChanged(uint32_t pos);
    void activityAdded(uint32_t pos);
    void activityAboutToBeRemoved(uint32_t pos);
    void activityRemoved(uint32_t pos);
    void dirtyChanged(bool dirty);

  public:
    bool addDeviceCommand(int pos, uint32_t id); //existing id
    bool addDeviceCommand(int pos, uint32_t *id = nullptr); //assign id
    bool removeDeviceCommand(int pos);

  protected:
    void connectCommand(BaseCommand *cmd);

  protected:
    ConfigData &c;
    lib::UndoStack &undo;
    lib::UidGenerator &uid;
};

}
}


