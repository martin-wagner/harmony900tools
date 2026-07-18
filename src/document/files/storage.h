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

    /** write config to disk at _workPath_ */
    bool write(const data::ConfigData &c);
    /** read files fom disk at _workPath_. be aware that this does not use the undo layer,
     * therefore you need to clean the stack afterwards */
    bool read(data::ConfigData &c);

  public:
    const QString jsonPath = "userconfig.json";
    const uint32_t jsonVersion = 1;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    bool readUserConfigJson(data::ConfigData &c);

  protected:
    bool writeUserConfigJson(const data::ConfigData &c);

  protected:
    const QString wp;
};

}
}


