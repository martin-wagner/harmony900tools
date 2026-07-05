// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <unordered_set>

#include "ui/logViewer.h"
#include "document/data/items/unknown.h"
#include "document/data/enum.h"

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
    bool readDeviceButtons(pugi::xml_node &buttons, enum data::item::ButtonType t);
    bool readDeviceButton(pugi::xml_node &button, enum data::item::ButtonType t);
    bool readStatemachines(pugi::xml_node &states);
    bool readStatemachine(pugi::xml_node &state);
    bool readDiscreteActions(pugi::xml_node &actions);
    bool readDiscreteAction(pugi::xml_node &action);
    bool readDiscreteActionSequences(pugi::xml_node &action);
    bool readDiscreteActionSequence(pugi::xml_node &sequence);
    bool readRelativeActions(pugi::xml_node &state);
    bool readRelativeAction(pugi::xml_node &action);
    bool readRelativeActionSequences(pugi::xml_node &action, data::item::StateTransitionAction t);
    bool readRelativeActionSequence(pugi::xml_node &sequence, data::item::StateTransitionAction t);
    bool readActionSequenceData(pugi::xml_node &sequence, uint32_t devicePos, uint32_t smPos, uint32_t actPos, data::item::StateTransitionAction t, uint32_t seqPos);
    bool readNumeric(pugi::xml_node &numeric);
    bool readNumericActions(pugi::xml_node &actions, data::item::DigitSection s);
    bool readNumericActionSequences(pugi::xml_node &action, uint32_t devicePos, data::item::DigitSection s, uint32_t digit);
    bool readNumericActionSequence(pugi::xml_node &sequence, uint32_t devicePos, data::item::DigitSection s, uint32_t digit);
    bool readActionSequenceData(pugi::xml_node &sequence, uint32_t devicePos, data::item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool readIrList(pugi::xml_node &commands);
    bool readIr(pugi::xml_node &command);
    bool readActivities(pugi::xml_node &root);
    bool readActivitiy(pugi::xml_node &activity, uint32_t id);
    bool readActivityChannels(pugi::xml_node &channels);
    bool readActivityChannel(pugi::xml_node &channel);
    bool readActivityButtons(pugi::xml_node &buttons, enum data::item::ButtonType t);
    bool readActivityButton(pugi::xml_node &button, enum data::item::ButtonType t);
    bool readActivityAction(pugi::xml_node &actions, data::item::ActivityAction t);
    bool readActivityActionSequences(pugi::xml_node &action, data::item::ActivityAction t);
    bool readActivityActionSequence(pugi::xml_node &sequence, data::item::ActivityAction t);
    bool readActivityActionSequenceData(pugi::xml_node &sequence, uint32_t activityPos, uint32_t actionPos, data::item::ActivityAction t, uint32_t seqPos);

    bool readProtocols(pugi::xml_node &root);
    bool readProtocol(pugi::xml_node &protocol);

    data::item::UnknownElement toUnknownElement(const pugi::xml_node& node);
    void addId(uint32_t id);

    bool readIrProto();
    bool readIrStream();

  protected:
    bool dumpUserConfigXml();
    bool writeProperties(pugi::xml_node &root);
    bool writeUser(pugi::xml_node &root);
    bool writeController(pugi::xml_node &root);
    bool writeDevices(pugi::xml_node &root);
    bool writeDevice(pugi::xml_node &device, const data::item::Device &data);
    bool writeDeviceButtons(pugi::xml_node &buttons, uint32_t deviceId, const std::vector<data::item::Button> &data, enum data::item::ButtonType t);
    bool writeDeviceButton(pugi::xml_node &button, uint32_t deviceId, const data::item::Button &data);
    bool writeStatemachines(pugi::xml_node &states, uint32_t deviceId, const std::vector<data::item::StateMachine> &data);
    bool writeStatemachine(pugi::xml_node &state, uint32_t deviceId, const data::item::StateMachine &data);
    bool writeDiscreteActions(pugi::xml_node &action, uint32_t deviceId, const data::item::DiscreteActions &data);
    bool writeRelativeActions(pugi::xml_node &action, uint32_t deviceId, const data::item::RelativeActions &data);
    bool writeDeviceAction(pugi::xml_node &actionType, uint32_t deviceId, const data::item::DeviceAction &data, data::item::StateTransitionType t);
    bool writeNumeric(pugi::xml_node &numeric, uint32_t deviceId, const data::item::Numpad &data);
    bool writeNumericActions(pugi::xml_node &action, uint32_t deviceId, const data::item::Digits &data);
    bool writeIrList(pugi::xml_node &commands, const data::item::Commands &data);
    bool writeIr(pugi::xml_node &command, const data::item::RawCommand &data);
    bool writeIr(pugi::xml_node &command, const data::item::ProtoCommand &data);
    bool writeActivities(pugi::xml_node &root);
    bool writeActivity(pugi::xml_node &activity, const data::item::Activity &data);
    bool writeActivityChannels(pugi::xml_node &channels, const std::vector<data::item::Channel> &data);
    bool writeActivityChannel(pugi::xml_node &channel, const data::item::Channel &data);
    bool writeActivityButtons(pugi::xml_node &buttons, const std::vector<data::item::Button> &data, enum data::item::ButtonType t);
    bool writeActivityButton(pugi::xml_node &button, const data::item::Button &data);
    bool writeActivityActions(pugi::xml_node &action, const std::vector<data::item::DeviceAction> &data);
    bool writeActivityAction(pugi::xml_node &actionType, const data::item::DeviceAction &data);


    bool writeProtocols(pugi::xml_node &root);
    bool writeProtocol(pugi::xml_node &protocol);

    void writeUnknownElement(pugi::xml_node& parent, const data::item::UnknownElement& element);

    bool dumpActionListXml();
    bool exportDevices(pugi::xml_node &root);
    bool exportDevice(pugi::xml_node &root, const data::item::Device &data);
    bool exportButtons(pugi::xml_node &root, uint32_t deviceId, const std::vector<data::item::Button> &data);
    bool exportButton(pugi::xml_node &root, uint32_t deviceId, const data::item::Button &data);

    bool writeIrProto();
    bool writeIrStream();

  protected:
    const QString wp;

  private:
    const data::ConfigData *c = nullptr;
    data::CmdCatalogue *worker = nullptr;
    std::string writerTime;
    std::string hash;
};

}
}


