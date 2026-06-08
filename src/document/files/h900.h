// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <unordered_set>

#include "ui/logViewer.h"
#include "document/data/items/unknown.h"

namespace pugi
{
  class xml_document;
  class xml_node;
  class xml_attribute;
  class xml_text;
}

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

    bool dump(const data::ConfigData *c);
    bool read(const data::ConfigData *c, data::CmdCatalogue *worker);

  public:
    const QString actionListPath = "userconfig/ActionLists.xml";
    const QString userConfigPath = "userconfig/UserConfiguration.xml";
    const QString irProtoPath = "userconfig/IrProto.bin";
    const QString ssIrPath = "userconfig/SsIr.bin";

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    bool readUserConfigXml();
    bool readProperties(pugi::xml_node &root);
    bool readUser(pugi::xml_node &root);
    bool readController(pugi::xml_node &root);
    bool readDevices(pugi::xml_node &root);
    bool readDevice(pugi::xml_node &device);
    bool readActivities(pugi::xml_node &root);
    bool readActivitiy(pugi::xml_node &activitie);
    bool readProtocols(pugi::xml_node &root);
    bool readProtocol(pugi::xml_node &protocol);

    data::item::UnknownElement toUnknownElement(const pugi::xml_node& node);
    void addId(uint32_t id);

  protected:
    bool dumpUserConfigXml();
    bool writeProperties(pugi::xml_node &root);
    bool writeUser(pugi::xml_node &root);
    bool writeController(pugi::xml_node &root);
    bool writeDevices(pugi::xml_node &root);
    bool writeDevice(pugi::xml_node &device, const data::item::Device &data, uint32_t pos);
    bool writeActivities(pugi::xml_node &root);
    bool writeActivitiy(pugi::xml_node &activitie);
    bool writeProtocols(pugi::xml_node &root);
    bool writeProtocol(pugi::xml_node &protocol);

    void writeUnknownElement(pugi::xml_node& parent, const data::item::UnknownElement& element);

  protected:
    const QString wp;

  private:
    const data::ConfigData *c = nullptr;
    data::CmdCatalogue *worker = nullptr;

};

}
}


