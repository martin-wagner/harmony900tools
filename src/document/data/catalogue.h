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

    void itemChanged(Item item, uint32_t pos);
    void itemAboutToBeAdded(Item item, uint32_t pos);
    void itemAdded(Item item, uint32_t pos);
    void itemAboutToBeRemoved(Item item, uint32_t pos);
    void itemRemoved(Item item, uint32_t pos);
    void dirtyChanged(bool dirty);

  public:
    //user
    bool setUserId(uint32_t id);
    bool setUserFirstName(const QString &v);
    bool setUserLastName(const QString &v);
    bool setUserMetadata();
    bool setUserMetadata(const QString &user, const QString &creationDate, const QString &modificationDate);
    bool setUserNewDeviceFound(bool f);
    bool setUserTrainingWheels(bool w);
    bool setUserLocale(const Enum<Locale> &l);
    bool setUserTimeFormat(const Enum<TimeFormat> &f);
    bool setUserUnknownProperty(const data::item::UnknownElement& value);
    //controller
    bool setControllerId(uint32_t id);
    bool setControllerMetadata(const QString &type, const QString &mnf, const QString &model, const QString &label);
    bool setControllerUnknownProperty(const data::item::UnknownElement& value);
    //device
    bool addDeviceCommand(int pos, uint32_t id); //existing id
    bool addDeviceCommand(int pos, uint32_t *id = nullptr); //assign id
    bool removeDeviceCommand(int pos);
    bool setDeviceType(const Enum<DeviceType> &v, uint32_t pos);
    bool setDeviceMnf(const QString &v, uint32_t pos);
    bool setDeviceModel(const QString &v, uint32_t pos);
    bool setDeviceLabel(const QString &v, uint32_t pos);
    bool setDeviceManualPower(bool v, uint32_t pos);
    bool setDeviceAlwaysOn(bool v, uint32_t pos);
    bool setDeviceAutoPower(bool v, uint32_t pos);
    bool setDeviceAudioSwitch(bool v, uint32_t pos);
    bool setDeviceDimmer(bool v, uint32_t pos);
    bool setDeviceHasBands(bool v, uint32_t pos);
    bool setDeviceHasPresets(bool v, uint32_t pos);
    bool setDeviceIsNewDevice(bool v, uint32_t pos);
    bool setDeviceIsDisplayDevice(bool v, uint32_t pos);
    bool setDeviceMenuOnDevice(bool v, uint32_t pos);
    bool setDeviceOnScreenGuide(bool v, uint32_t pos);
    bool setDeviceRecordMediaFixedDisc(bool v, uint32_t pos);
    bool setDeviceRecordMediaRemovableVideotape(bool v, uint32_t pos);
    bool setDeviceRevertInput(bool v, uint32_t pos);
    bool setDeviceScart(bool v, uint32_t pos);
    bool setDeviceVideoSwitch(bool v, uint32_t pos);
    bool setDeviceNumDiscs(int32_t v, uint32_t pos);
    bool setDeviceNumLights(int32_t v, uint32_t pos);
    bool setDevicePvrType(const Enum<PvrType> &v, uint32_t pos);
    bool setDeviceTunerInput(const Enum<TunerInput> &v, uint32_t pos);
    bool setDeviceUnknownProperty(const data::item::UnknownElement& value, uint32_t pos);
    //buttons
    bool addDeviceButtonCommand(uint32_t devicePos, item::ButtonType t, int buttonPos);
    bool removeDeviceButtonCommand(uint32_t devicePos, item::ButtonType t, int buttonPos);
    bool setDeviceButtonAction(const std::string &v, uint32_t devicePos, item::ButtonType t, int buttonPos);
    bool setDeviceButtonName(const std::string &v, uint32_t devicePos, item::ButtonType t, int buttonPos);
    bool setDeviceButtonFile(const std::string &v, uint32_t devicePos, item::ButtonType t, int buttonPos);
    bool setDeviceButtonPosition(const int32_t &v, uint32_t devicePos, item::ButtonType t, int buttonPos);
    //state machines
    bool addDeviceStatemachineCommand(uint32_t devicePos, int smPos);
    bool removeDeviceStatemachineCommand(uint32_t devicePos, int smPos);
    bool setDeviceStatemachineType(const Enum<StateMachineDeviceType> &type,  uint32_t devicePos, int smPos);
    bool setDeviceStatemachineDelay(uint32_t delayMs, uint32_t devicePos, int smPos);
    //state machine states/actions
    bool addDeviceSmStateCommand(uint32_t devicePos, uint32_t smPos, item::StateMachineType t, const QString &name, int actPos);
    bool addDeviceSmActionCommand(uint32_t devicePos, uint32_t smPos, item::StateMachineAction t);
    bool removeDeviceSmStateCommand(uint32_t devicePos, uint32_t smPos, item::StateMachineType t, int actPos);
    bool removeDeviceSmActionCommand(uint32_t devicePos, uint32_t smPos, item::StateMachineAction t);
    bool setDeviceSmActionType(const Enum<ActionType> &v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos);
    bool setDeviceSmActionRepeatWillNotHarm(bool v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos);
    //state machine action list items
    bool addDeviceStateActionSequenceCommand(uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateMachineAction t, int seqPos); //only discrete actions can add/remove
    bool removeDeviceStateActionSequenceCommand(uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateMachineAction t, uint32_t seqPos); //only discrete actions can add/remove
    bool setDeviceStateActionSequenceOp(const Enum<Operation> &v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceCmd(const std::string &v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceDelayMs(uint32_t v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceStateName(const Enum<StateMachineDeviceType> &v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceStateValue(const std::string &v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceMod(const Enum<Modifier> &v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionUnknownParam(const data::item::UnknownElement& v, uint32_t devicePos, uint32_t smPos, item::StateMachineAction t, uint32_t actPos, uint32_t seqPos);
    //numpad
    bool addDeviceNumpadCommand(uint32_t devicePos);
    bool removeDeviceNumpadCommand(uint32_t devicePos);
    bool addDeviceNumpadDigitsCommand(uint32_t devicePos, item::DigitSection s);
    bool removeDeviceNumpadDigitsCommand(uint32_t devicePos, item::DigitSection s);
    bool setDeviceNumpadFixedDigits(uint32_t v, uint32_t devicePos);
    //numpad actions
    bool setDeviceNumpadActionRepeatWillNotHarm(bool v, uint32_t devicePos, item::DigitSection s, uint32_t digit = 0);
    //numpad action list items
    bool addDeviceNumpadActionSequenceCommand(uint32_t devicePos, item::DigitSection s, uint32_t digit, int seqPos);
    bool removeDeviceNumpadActionSequenceCommand(uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceOp(const Enum<Operation> &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceCmd(const std::string &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceDelayMs(uint32_t v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceStateName(const Enum<StateMachineDeviceType> &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceStateValue(const std::string &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceMod(const Enum<Modifier> &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionUnknownParam(const data::item::UnknownElement& value, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    //commands
    bool setIrPressPreSilenceMs(uint32_t v, uint32_t devicePos);
    bool setIrPressInterKeyMs(uint32_t v, uint32_t devicePos);
    bool setIrHoldPreSilenceMs(uint32_t v, uint32_t devicePos);
    bool setIrHoldInterKeyMs(uint32_t v, uint32_t devicePos);
    bool setIrUnknownProperty(const data::item::UnknownElement& value, uint32_t devicePos);
    bool setIrCommand(uint32_t devicePos, item::ProtoCommand &cmd, int cmdPos = -1, bool overwrite = false);
    bool setIrCommand(uint32_t devicePos, item::RawCommand &cmd, int cmdPos = -1, bool overwrite = false);
    bool removeIrProtoCommand(uint32_t devicePos, uint32_t cmdPos);
    bool removeIrRawCommand(uint32_t devicePos, uint32_t cmdPos);
    //activity
    bool addActivityCommand(int pos, uint32_t id); //existing id
    bool addActivityCommand(int pos, uint32_t *id = nullptr); //assign id
    bool moveActivityCommand(uint32_t currentPos, uint32_t newPos);
    bool removeActivityCommand(int pos);
    bool setActivityType(const Enum<ActivityType> &v, uint32_t pos);
    bool setActivityLabel(const QString &v, uint32_t pos);
    bool setActivityPvrType(const Enum<ActivityStartPage> &v, uint32_t pos);
    bool setActivityControlGroupHardButtons(bool v, uint32_t pos);
    bool setActivityPowerOffUnusedDevices(bool v, uint32_t pos);
    bool setActivityTrainingWheels(bool v, uint32_t pos);
    bool setActivityUnusedDevicesHelp(bool v, uint32_t pos);
    bool setActivityChannelButtonBehaviour(const Enum<ChannelButtonBehaviour> &v, uint32_t pos);
    bool setActivityControlGroupSoftButtons(bool v, uint32_t pos);
    bool setActivityEnableSmartMenu(bool v, uint32_t pos);
    bool setActivityEnableSmartZoom(bool v, uint32_t pos);
    bool setActivityGuideButtonMode(const Enum<GuideButtonMode> &v, uint32_t pos);
    bool setActivityHideModeControl(bool v, uint32_t pos);
    bool setActivityHideModeListen(bool v, uint32_t pos);
    bool setActivityHideModeNavigate(bool v, uint32_t pos);
    bool setActivityHideModePlay(bool v, uint32_t pos);
    bool setActivityHideModePlayMode(bool v, uint32_t pos);
    bool setActivityHideSurfAllChannels(bool v, uint32_t pos);
    bool setActivityHideSurfAllShows(bool v, uint32_t pos);
    bool setActivityHideSurfFavoriteChannels(bool v, uint32_t pos);
    bool setActivityHideSurfFavoriteShows(bool v, uint32_t pos);
    bool setActivityMaxTvContentDays(int32_t v, uint32_t pos);
    bool setActivityMediaButtonMode(const Enum<MediaButtonMode> &v, uint32_t pos);
    bool setActivityPlayOnEnter(bool v, uint32_t pos);
    bool setActivityRetainStop(bool v, uint32_t pos);
    bool setActivityScrollChannelsByPage(bool v, uint32_t pos);
    bool setActivityScrollShowsByPage(bool v, uint32_t pos);
    bool setActivityStopOnExit(bool v, uint32_t pos);
    bool setActivityUnknownProperty(const data::item::UnknownElement& value, uint32_t pos);
    //channels
    bool addActivityChannelCommand(uint32_t activityPos, int chPos);
    bool removeActivityChannelCommand(uint32_t activityPos, uint32_t chPos);
    bool setActivityChannelStation(const std::string &v, uint32_t activityPos, uint32_t chPos);
    bool setActivityChannelNumber(const uint32_t v, uint32_t activityPos, uint32_t chPos);
    bool setActivityChannelPosition(const uint32_t v, uint32_t activityPos, uint32_t chPos);
    bool setActivityChannelImage(const std::string &v, uint32_t activityPos, uint32_t chPos);
    //buttons
    bool addActivityButtonCommand(uint32_t activityPos, item::ButtonType t, int buttonPos);
    bool removeActivityButtonCommand(uint32_t activityPos, item::ButtonType t, int buttonPos);
    bool setActivityButtonAction(const std::string &v, uint32_t activityPos, item::ButtonType t, int buttonPos);
    bool setActivityButtonDevice(uint32_t v, uint32_t activityPos, item::ButtonType t, int buttonPos);
    bool setActivityButtonName(const std::string &v, uint32_t activityPos, item::ButtonType t, int buttonPos);
    bool setActivityButtonFile(const std::string &v, uint32_t activityPos, item::ButtonType t, int buttonPos);
    bool setActivityButtonPosition(const int32_t v, uint32_t activityPos, item::ButtonType t, int buttonPos);
    //activity actions
    bool addActivityActionCommand(uint32_t activityPos, item::ActivityAction t, int actionPos);
    bool removActivityActionCommand(uint32_t activityPos,  item::ActivityAction t, int actionPos);
    //activity action list items
    bool addActivityActionSequenceCommand(uint32_t activityPos,  item::ActivityAction t, int actionPos, int seqPos);
    bool removeActivityActionSequenceCommand(uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionSequenceOp(const Enum<Operation> &v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionSequenceCmd(const std::string &v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionSequenceDeviceId(uint32_t v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionSequenceDelayMs(uint32_t v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionSequenceStateName(const Enum<StateMachineDeviceType> &v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionSequenceStateValue(const std::string &v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionSequenceMod(const Enum<Modifier> &v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    bool setActivityActionUnknownParam(const data::item::UnknownElement& v, uint32_t activityPos,  item::ActivityAction t, int actionPos, uint32_t seqPos);
    //activity roles
    bool setActivityRoleCommand(uint32_t activityPos, item::Role &role, int rolePos = -1, bool overwrite = false);
    bool removeActivityRoleCommand(uint32_t activityPos, uint32_t rolePos);
    bool removeActivityRoleCommandById(uint32_t activityPos, uint32_t deviceId);
    //activity power on devices
    bool addActivityPowerOnDevicesCommand(uint32_t activityPos, uint32_t id, int devicePos = -1);
    bool removeActivityPowerOnDevicesCommand(uint32_t activityPos, uint32_t devicePos);
    bool removeActivityPowerOnDevicesCommandById(uint32_t activityPos, uint32_t deviceId);
    bool addActivityPowerOffDevicesCommand(uint32_t activityPos, uint32_t id, int devicePos = -1);
    bool removeActivityPowerOffDevicesCommand(uint32_t activityPos, uint32_t devicePos);
    bool removeActivityPowerOffDevicesCommandById(uint32_t activityPos, uint32_t deviceId);

    //irproto binary lib. lib can be added and protocols appended.
    //items can't be removed, as this would require re-mapping of all consumers (not implemented)
    bool setIrProtoLib(const binary::irProto::File &file);
    int appendIrProtoLibItem(const Enum<CodeType> &t); //check if exists, append if not

  protected:
    //template for setting a property inside user data that uses the Property<x> template.
    //all property setters above use this.
    template<typename T>
    struct PropertyAccess {
        QString description;
        std::function<T()> getter;
        std::function<void(const T&)> setter;
        T value;
        Item changedItem = Item::UNKNOWN;
        uint32_t changedPos = 0;
    };
    template<typename T>
    bool setProperty(const PropertyAccess<T> &access)
    {
      auto *cmd = new SetPropertyBaseCommand<T>(access.description,
          access.getter, access.setter, access.value);
      auto ret = cmd->valid();
      if (ret == true) {
        cmd->setChangedSignal(access.changedItem, access.changedPos);
        connectCommand(cmd);
        undo.push(cmd);
      } else {
        emit writeLog(LogLevel::Warning,
            tr("modify: setting property failed (%1)").arg(access.description),
            ContentType::PlainText);
        delete cmd;
      }
      return ret;
    }

  protected:
    void connectCommand(BaseCommand *cmd);

  protected:
    ConfigData &c;
    lib::UndoStack &undo;
    lib::UidGenerator &uid;

};

}
}


