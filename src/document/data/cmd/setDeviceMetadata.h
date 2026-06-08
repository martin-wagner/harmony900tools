// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetDeviceMetadataCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetDeviceMetadataCommand(ConfigData &c, const Enum<DeviceType> &type,
        const QString &mnf, const QString &model, const QString &label,
        uint32_t pos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    ConfigData &c;
    uint32_t pos;

    Enum<DeviceType> type;
    std::string mnf;
    std::string model;
    std::string label;
    Enum<DeviceType> prevType;
    std::string prevMnf;
    std::string prevModel;
    std::string prevLabel;
};

class SetDeviceTypeCommand: public SetPropertyBaseCommand<Enum<DeviceType>>
{
  public:
    SetDeviceTypeCommand(ConfigData &c, const Enum<DeviceType> &value,
        uint32_t pos, QUndoCommand *parent = nullptr);
};

class SetDeviceMnfCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetDeviceMnfCommand(ConfigData &c, const QString &value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceModelCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetDeviceModelCommand(ConfigData &c, const QString &value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceLabelCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetDeviceLabelCommand(ConfigData &c, const QString &value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceManualPowerCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceManualPowerCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceAlwaysOnCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceAlwaysOnCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceAutoPowerCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceAutoPowerCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceAudioSwitchCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceAudioSwitchCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceDimmerCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceDimmerCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceHasBandsCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceHasBandsCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceHasPresetsCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceHasPresetsCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceIsNewDeviceCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceIsNewDeviceCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceIsDisplayDeviceCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceIsDisplayDeviceCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceMenuOnDeviceCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceMenuOnDeviceCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceNumDiscsCommand: public SetPropertyBaseCommand<int32_t>
{
  public:
    SetDeviceNumDiscsCommand(ConfigData &c, int32_t value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceNumLightsCommand: public SetPropertyBaseCommand<int32_t>
{
  public:
    SetDeviceNumLightsCommand(ConfigData &c, int32_t value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceOnScreenGuideCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceOnScreenGuideCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDevicePvrTypeCommand: public SetPropertyBaseCommand<Enum<PvrType>>
{
  public:
    SetDevicePvrTypeCommand(ConfigData &c, const Enum<PvrType> &value,
        uint32_t pos, QUndoCommand *parent = nullptr);
};

class SetDeviceRecordMediaFixedDiscCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceRecordMediaFixedDiscCommand(ConfigData &c, bool value,
        uint32_t pos, QUndoCommand *parent = nullptr);
};

class SetDeviceRecordMediaRemovableVideotapeCommand: public SetPropertyBaseCommand<
    bool>
{
  public:
    SetDeviceRecordMediaRemovableVideotapeCommand(ConfigData &c, bool value,
        uint32_t pos, QUndoCommand *parent = nullptr);
};

class SetDeviceRevertInputCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceRevertInputCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceScartCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceScartCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

class SetDeviceTunerInputCommand: public SetPropertyBaseCommand<Enum<TunerInput>>
{
  public:
    SetDeviceTunerInputCommand(ConfigData &c, const Enum<TunerInput> &value,
        uint32_t pos, QUndoCommand *parent = nullptr);
};

class SetDeviceVideoSwitchCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetDeviceVideoSwitchCommand(ConfigData &c, bool value, uint32_t pos,
        QUndoCommand *parent = nullptr);
};

}
}
