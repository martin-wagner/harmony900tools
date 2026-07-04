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
    void deviceAboutToBeAdded(uint32_t pos);
    void deviceAdded(uint32_t pos);
    void deviceAboutToBeRemoved(uint32_t pos);
    void deviceRemoved(uint32_t pos);
    void activityChanged(uint32_t pos);
    void activityAboutToBeAdded(uint32_t pos);
    void activityAdded(uint32_t pos);
    void activityAboutToBeRemoved(uint32_t pos);
    void activityRemoved(uint32_t pos);
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
    bool addDeviceButtonCommand(item::ButtonType t, uint32_t devicePos, int buttonPos);
    bool removeDeviceButtonCommand(uint32_t devicePos, int buttonPos);
    bool setDeviceButtonAction(const std::string &v, uint32_t devicePos, int buttonPos);
    bool setDeviceButtonName(const std::string &v, uint32_t devicePos, int buttonPos);
    bool setDeviceButtonFile(const std::string &v, uint32_t devicePos, int buttonPos);
    bool setDeviceButtonPosition(const int32_t &v, uint32_t devicePos, int buttonPos);
    //state machines
    bool addDeviceStatemachineCommand(uint32_t devicePos, int smPos);
    bool removeDeviceStatemachineCommand(uint32_t devicePos, int smPos);
    bool setDeviceStatemachineType(const Enum<StateMachineType> &type,  uint32_t devicePos, int smPos);
    bool setDeviceStatemachineDelay(uint32_t delayMs, uint32_t devicePos, int smPos);
    //state machine states/actions
    bool addDeviceSmStateCommand(uint32_t devicePos, uint32_t smPos, item::StateTransitionType t, const QString &name, int actPos);
    bool addDeviceSmActionCommand(uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t);
    bool removeDeviceSmStateCommand(uint32_t devicePos, uint32_t smPos, item::StateTransitionType t, int actPos);
    bool removeDeviceSmActionCommand(uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t);
    bool setDeviceSmActionType(const Enum<ActionType> &v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos);
    bool setDeviceSmActionRepeatWillNotHarm(bool v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos);
    //state machine action list items
    bool addDeviceStateActionSequenceCommand(uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateTransitionAction t, int seqPos); //only discrete actions can add/remove
    bool removeDeviceStateActionSequenceCommand(uint32_t devicePos, uint32_t smPos, uint32_t actPos, item::StateTransitionAction t, uint32_t seqPos); //only discrete actions can add/remove
    bool setDeviceStateActionSequenceOp(const Enum<Operation> &v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceCmd(const std::string &v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceDelayMs(uint32_t v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceStateName(const Enum<StateMachineType> &v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceStateValue(const std::string &v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionSequenceMod(const Enum<Modifier> &v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos);
    bool setDeviceStateActionUnknownParam(const data::item::UnknownElement& v, uint32_t devicePos, uint32_t smPos, item::StateTransitionAction t, uint32_t actPos, uint32_t seqPos);
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
    bool setDeviceNumpadActionSequenceStateName(const Enum<StateMachineType> &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceStateValue(const std::string &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionSequenceMod(const Enum<Modifier> &v, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    bool setDeviceNumpadActionUnknownParam(const data::item::UnknownElement& value, uint32_t devicePos, item::DigitSection s, uint32_t digit, uint32_t seqPos);
    //commands
    bool setIrPressPreSilenceMs(uint32_t v, uint32_t devicePos);
    bool setIrPressInterKeyMs(uint32_t v, uint32_t devicePos);
    bool setIrHoldPreSilenceMs(uint32_t v, uint32_t devicePos);
    bool setIrHoldInterKeyMs(uint32_t v, uint32_t devicePos);
    bool setIrCodeType(const Enum<CodeType> &v, uint32_t devicePos);
    bool setIrCodeField0(uint32_t v, uint32_t devicePos);
    bool setIrCodeField1(uint32_t v, uint32_t devicePos);
    bool setIrCodeField2(uint32_t v, uint32_t devicePos);
    bool setIrUnknownProperty(const data::item::UnknownElement& value, uint32_t devicePos);
    bool setIrCommand(uint32_t devicePos, item::ProtoCommand &cmd, int cmdPos = -1, bool overwrite = false);
    bool setIrCommand(uint32_t devicePos, item::RawCommand &cmd, int cmdPos = -1, bool overwrite = false);
    bool removeIrProtoCommand(uint32_t devicePos, uint32_t cmdPos);
    bool removeIrRawCommand(uint32_t devicePos, uint32_t cmdPos);
    //activity
    bool addActivityCommand(int pos, uint32_t id); //existing id
    bool addActivityCommand(int pos, uint32_t *id = nullptr); //assign id
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

    //irproto binary lib. items can be added/removed, but currently not directly modified/overwritten
    bool setIrProtoLib(const binary::irProto::File &file);
    bool addIrProtoLibItem(const binary::irProto::IrProto &prot, int pos = -1);
    bool removeIrProtoLibItem(int pos = -1);

    //irstream binary list. items can be added/removed, but currently not directly modified/overwritten
    bool setIrStreams(const binary::ssIr::File &file);
    bool addIrStreamItem(binary::TimingStream stream, double clock, int pos = -1);
    bool removeIrStreamItem(int pos = -1);

  protected:
    //template for setting a property inside user data that uses the Property<x> template.
    //all property setters above use this.
    template<typename T>
    struct PropertyAccess {
        QString description;
        std::function<T()> getter;
        std::function<void(const T&)> setter;
        T value;
    };
    template<typename T>
    bool setProperty(const PropertyAccess<T> &access)
    {
      auto *cmd = new SetPropertyBaseCommand<T>(access.description,
          access.getter, access.setter, access.value);
      auto ret = cmd->valid();
      if (ret == true) {
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


