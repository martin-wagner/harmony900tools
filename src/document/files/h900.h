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

class ConfigH900 : public QObject
{
  Q_OBJECT
  public:
    ConfigH900(const QString &workPath);

    bool dump(const data::ConfigData &c);
    bool read(data::ConfigData &c, data::CmdCatalogue *worker);

  public:
    const QString actionListPath = "userconfig/ActionLists.xml";
    const QString userConfigPath = "userconfig/UserConfiguration.xml";
    const QString irProtoPath = "userconfig/IrProto.bin";
    const QString ssIrPath = "userconfig/SsIr.bin";

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);


  protected:
    bool readUserConfigXml(data::ConfigData &c, data::CmdCatalogue *worker);

  protected:
    const QString wp;

};

}
}


