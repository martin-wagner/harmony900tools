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

/** this class writes the default setup for a new project */
class Create : public QObject
{
  Q_OBJECT
  public:
    Create(data::ConfigData &c);

    /** write default data */
    void write(data::CmdCatalogue *worker);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    void addId(uint32_t id);

  protected:
    data::ConfigData &c;
    data::CmdCatalogue *worker = nullptr;
    std::string writerTime;
};

}
}


