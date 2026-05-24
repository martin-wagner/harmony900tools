// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "ui/logViewer.h"

namespace document
{

namespace data {
  class ConfigData;
  class CmdCatalogue;
}

namespace files
{

class ConfigStorage : public QObject
{
  Q_OBJECT
  public:
    ConfigStorage(const QString &workPath);

    bool write(const data::ConfigData &c);
    bool read(data::ConfigData &c, data::CmdCatalogue *worker);

  public:
    const QString jsonPath = "userconfig.json";
    const uint32_t jsonVersion = 1;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    bool readUserConfigJson(data::ConfigData &c, data::CmdCatalogue *worker);

  protected:
    bool writeUserConfigJson(const data::ConfigData &c);

  protected:
    const QString wp;
};

}
}


