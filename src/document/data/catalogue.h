// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "lib/undo.h"
#include "lib/uid.h"
#include "ui/logViewer.h"

namespace document
{
namespace data
{

class Config;

/** all data modification commands, implementing dependencies and undo */
class CmdCatalogue : public QObject
{
  Q_OBJECT
  public:
    CmdCatalogue(Config &c, lib::UndoStack &undo, QObject *parent = nullptr);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  public:
    bool addDeviceCommand(uint32_t *id);
    bool removeDeviceCommand(uint32_t id);

  protected:
    Config &c;
    lib::UndoStack &undo;
    lib::UidGenerator &uid;
};

}
}


