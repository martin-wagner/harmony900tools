// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "ui/logViewer.h"

namespace document
{

class Config;
namespace data {
  class ConfigData;
  class CmdCatalogue;
}

namespace files
{

class DeviceStorage : public QObject
{
  Q_OBJECT
  public:
    DeviceStorage();

    /** write single device to disk */
    bool write(const data::ConfigData &c, int deviceId);
    /** import device fom disk */
    bool import(Config &c);

  public:
    const uint32_t jsonVersion = 1;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    bool importDeviceJson(Config &c);

  protected:
    bool writeDeviceJson(const data::ConfigData &c, int deviceId);
};

}
}


