// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setDeviceMetadata.h"

using namespace std;

namespace document
{
namespace data
{

SetDeviceMetadataCommand::SetDeviceMetadataCommand(ConfigData &c,
    const Enum<DeviceType> &type, const QString &mnf, const QString &model,
    const QString &label, uint32_t pos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set controller metadata)"), parent), c(c), pos(
        pos), type(type), mnf(mnf.toStdString()), model(model.toStdString()), label(
        label.toStdString()), prevType(c.getDevices()[pos].type.get())
{
  //prevType -- init list
  prevMnf = c.getDevices()[pos].mnf.get();
  prevModel = c.getDevices()[pos].model.get();
  prevLabel = c.getDevices()[pos].label.get();
}

void SetDeviceMetadataCommand::redo()
{
  c.getDevices()[pos].type.set(type);
  c.getDevices()[pos].mnf.set(mnf);
  c.getDevices()[pos].model.set(model);
  c.getDevices()[pos].label.set(label);
  emit dirtyChanged(true);
}

void SetDeviceMetadataCommand::undo()
{
  c.getDevices()[pos].type.set(prevType);
  c.getDevices()[pos].mnf.set(prevMnf);
  c.getDevices()[pos].model.set(prevModel);
  c.getDevices()[pos].label.set(prevLabel);
  emit dirtyChanged(true);
}

SetDeviceTypeCommand::SetDeviceTypeCommand(ConfigData &c,
    const Enum<DeviceType> &value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<DeviceType>>(QObject::tr("set device type"),
        [&c, pos]() {return c.getDevices()[pos].type.get();},
        [&c, pos](const Enum<DeviceType> &v) {
          c.getDevices()[pos].type.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceMnfCommand::SetDeviceMnfCommand(ConfigData &c, const QString &value,
    uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set manufacturer"),
        [&c, pos]() {return c.getDevices()[pos].mnf.get();},
        [&c, pos](const string &v) {
          c.getDevices()[pos].mnf.set(v).setIncluded(Include::ALWAYS);
        }, value.toStdString(), parent)
{
}

SetDeviceModelCommand::SetDeviceModelCommand(ConfigData &c,
    const QString &value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set model"),
        [&c, pos]() {return c.getDevices()[pos].model.get();},
        [&c, pos](const string &v) {
          c.getDevices()[pos].model.set(v).setIncluded(Include::ALWAYS);
        }, value.toStdString(), parent)
{
}

SetDeviceLabelCommand::SetDeviceLabelCommand(ConfigData &c,
    const QString &value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set label"),
        [&c, pos]() {return c.getDevices()[pos].label.get();},
        [&c, pos](const string &v) {
          c.getDevices()[pos].label.set(v).setIncluded(Include::ALWAYS);
        }, value.toStdString(), parent)
{
}

SetDeviceManualPowerCommand::SetDeviceManualPowerCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set manual power"),
        [&c, pos]() {return c.getDevices()[pos].manualPower.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].manualPower.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceAlwaysOnCommand::SetDeviceAlwaysOnCommand(ConfigData &c, bool value,
    uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set always on"),
        [&c, pos]() {return c.getDevices()[pos].alwaysOn.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].alwaysOn.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceAutoPowerCommand::SetDeviceAutoPowerCommand(ConfigData &c, bool value,
    uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set auto power"),
        [&c, pos]() {return c.getDevices()[pos].autoPower.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].autoPower.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceAudioSwitchCommand::SetDeviceAudioSwitchCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set audio switch"),
        [&c, pos]() {return c.getDevices()[pos].audioSwitch.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].audioSwitch.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceDimmerCommand::SetDeviceDimmerCommand(ConfigData &c, bool value,
    uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set dimmer"),
        [&c, pos]() {return c.getDevices()[pos].dimmer.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].dimmer.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceHasBandsCommand::SetDeviceHasBandsCommand(ConfigData &c, bool value,
    uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set has bands"),
        [&c, pos]() {return c.getDevices()[pos].hasBands.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].hasBands.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceHasPresetsCommand::SetDeviceHasPresetsCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set has presets"),
        [&c, pos]() {return c.getDevices()[pos].hasPresets.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].hasPresets.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceIsNewDeviceCommand::SetDeviceIsNewDeviceCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set is new device"),
        [&c, pos]() {return c.getDevices()[pos].isNewDevice.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].isNewDevice.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceIsDisplayDeviceCommand::SetDeviceIsDisplayDeviceCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set is display device"),
        [&c, pos]() {return c.getDevices()[pos].isDisplayDevice.get();},
        [&c, pos](
            const bool &v) {
              c.getDevices()[pos].isDisplayDevice.set(v).setIncluded(Include::ALWAYS);
            }, value, parent)
{
}

SetDeviceMenuOnDeviceCommand::SetDeviceMenuOnDeviceCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set menu on device"),
        [&c, pos]() {return c.getDevices()[pos].menuOnDevice.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].menuOnDevice.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceNumDiscsCommand::SetDeviceNumDiscsCommand(ConfigData &c, int32_t value,
    uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<int32_t>(QObject::tr("set number of discs"),
        [&c, pos]() {return c.getDevices()[pos].numDiscs.get();},
        [&c, pos](const int32_t &v) {
          c.getDevices()[pos].numDiscs.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceNumLightsCommand::SetDeviceNumLightsCommand(ConfigData &c,
    int32_t value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<int32_t>(QObject::tr("set number of lights"),
        [&c, pos]() {return c.getDevices()[pos].numLights.get();},
        [&c, pos](const int32_t &v) {
          c.getDevices()[pos].numLights.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceOnScreenGuideCommand::SetDeviceOnScreenGuideCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set on screen guide"),
        [&c, pos]() {return c.getDevices()[pos].onScreenGuide.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].onScreenGuide.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDevicePvrTypeCommand::SetDevicePvrTypeCommand(ConfigData &c,
    const Enum<PvrType> &value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<PvrType>>(QObject::tr("set pvr type"),
        [&c, pos]() {return c.getDevices()[pos].pvrType.get();},
        [&c, pos](const Enum<PvrType> &v) {
          c.getDevices()[pos].pvrType.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceRecordMediaFixedDiscCommand::SetDeviceRecordMediaFixedDiscCommand(
    ConfigData &c, bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set record media fixed disc"),
        [&c, pos]() {return c.getDevices()[pos].recordMediaFixedDisc.get();},
        [&c, pos](
            const bool &v) {
              c.getDevices()[pos].recordMediaFixedDisc.set(v).setIncluded(Include::ALWAYS);
            }, value, parent)
{
}

SetDeviceRecordMediaRemovableVideotapeCommand::SetDeviceRecordMediaRemovableVideotapeCommand(
    ConfigData &c, bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(
        QObject::tr("set record media removable videotape"), [&c, pos]() {
          return c.getDevices()[pos].recordMediaRemovableVideotape.get();
        }, [&c, pos](const bool &v) {
          c.getDevices()[pos].recordMediaRemovableVideotape
          .set(v)
          .setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceRevertInputCommand::SetDeviceRevertInputCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set revert input"),
        [&c, pos]() {return c.getDevices()[pos].revertInput.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].revertInput.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceScartCommand::SetDeviceScartCommand(ConfigData &c, bool value,
    uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set scart"),
        [&c, pos]() {return c.getDevices()[pos].scart.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].scart.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceTunerInputCommand::SetDeviceTunerInputCommand(ConfigData &c,
    const Enum<TunerInput> &value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<TunerInput>>(QObject::tr("set tuner input"),
        [&c, pos]() {return c.getDevices()[pos].tunerInput.get();},
        [&c, pos](const Enum<TunerInput> &v) {
          c.getDevices()[pos].tunerInput.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetDeviceVideoSwitchCommand::SetDeviceVideoSwitchCommand(ConfigData &c,
    bool value, uint32_t pos, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set video switch"),
        [&c, pos]() {return c.getDevices()[pos].videoSwitch.get();},
        [&c, pos](const bool &v) {
          c.getDevices()[pos].videoSwitch.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

}
}
