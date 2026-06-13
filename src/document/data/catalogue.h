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
    bool setUserName(const QString &firstName, const QString &lastName);
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
    bool setDeviceMetadata(const Enum<DeviceType> &type, const QString &mnf, const QString &model, const QString &label, uint32_t pos);
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
    bool setDeviceType(const Enum<DeviceType> &v, uint32_t pos);
    bool setDevicePvrType(const Enum<PvrType> &v, uint32_t pos);
    bool setDeviceTunerInput(const Enum<TunerInput> &v, uint32_t pos);
    bool setDeviceUnknownProperty(const data::item::UnknownElement& value, uint32_t pos);
    //buttons
    bool addButtonCommand(item::ButtonType t, uint32_t devicePos, int buttonPos);
    bool removeButtonCommand(item::ButtonType t, uint32_t devicePos, int buttonPos);
    bool setButtonAction(const std::string &v, item::ButtonType t, uint32_t devicePos, int buttonPos);
    bool setButtonName(const std::string &v, item::ButtonType t, uint32_t devicePos, int buttonPos);
    bool setButtonFile(const std::string &v, item::ButtonType t, uint32_t devicePos, int buttonPos);
    bool setButtonPosition(const int32_t &v, item::ButtonType t, uint32_t devicePos, int buttonPos);
    //state machines
    bool addStatemachineCommand(uint32_t devicePos, int smPos);
    bool removeStatemachineCommand(uint32_t devicePos, int smPos);
    bool setStatemachineType(const Enum<StateMachineType> &type,  uint32_t devicePos, int smPos);
    bool setStatemachineDelay(uint32_t delayMs, uint32_t devicePos, int smPos);
    bool setStatemachineActionClass(const Enum<ActionClass> &v, uint32_t devicePos, int smPos);
    //state machine actions
    bool addDeviceActionCommand(uint32_t devicePos, int smPos, uint32_t actPos);
    bool removeDeviceActionCommand(uint32_t devicePos, int smPos, uint32_t actPos);
    bool setDeviceActionType(const Enum<ActionType> &v, uint32_t devicePos, int smPos, uint32_t actPos);
    bool setDeviceActionName(const std::string &v, uint32_t devicePos, int smPos, uint32_t actPos);
    bool setDeviceActionRepeatWillNotHarm(const bool &v, uint32_t devicePos, int smPos, uint32_t actPos);
    bool setDeviceActionOp(const Enum<Operation> &v, uint32_t devicePos, int smPos, uint32_t actPos);
    bool setDeviceActionCmd(const std::string &v, uint32_t devicePos, int smPos, uint32_t actPos);
    bool setDeviceActionDelayMs(const uint32_t &v, uint32_t devicePos, int buttonPos, uint32_t actPos);
    bool setDeviceActionStateName(const Enum<StateMachineType> &v, uint32_t devicePos, int buttonPos, uint32_t actPos);
    bool setDeviceActionStateValue(const std::string &v, uint32_t devicePos, int buttonPos, uint32_t actPos);
    bool setDeviceActionMod(const Enum<Modifier> &v, uint32_t devicePos, int smPos, uint32_t actPos);
    bool setDeviceActionUnknownParam(const data::item::UnknownElement& value, uint32_t devicePos, int smPos, uint32_t actPos);

  protected:
    void connectCommand(BaseCommand *cmd);

  protected:
    ConfigData &c;
    lib::UndoStack &undo;
    lib::UidGenerator &uid;
};

}
}


