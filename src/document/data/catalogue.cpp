// SPDX-License-Identifier: LGPL-2.1-or-later

#include "catalogue.h"

#include "document/config.h"
#include "cmd/addDevice.h"
#include "cmd/removeDevice.h"
#include "cmd/setId.h"
#include "cmd/setUserMetadata.h"
#include "cmd/setControllerMetadata.h"
#include "cmd/setDeviceMetadata.h"
#include "cmd/setUserName.h"
#include "cmd/setUnknownProperty.h"

using namespace std;

namespace document
{
namespace data
{

CmdCatalogue::CmdCatalogue(ConfigData &c, lib::UndoStack &undo, QObject *parent) :
    QObject(parent), c(c), undo(undo), uid(lib::UidGenerator::getInstance())
{
}

bool CmdCatalogue::setUserId(uint32_t id)
{
  auto *cmd = new SetUserIdCommand(c, id);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserName(const QString &firstName,
    const QString &lastName)
{
  auto *cmd = new SetUserNameCommand(c, firstName, lastName);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserMetadata()
{
  auto *cmd = new SetUserMetadataCommand(c);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserMetadata(const QString &user,
    const QString &creationDate, const QString &modificationDate)
{
  auto *cmd = new SetUserMetadataCommand(c, user, creationDate,
      modificationDate);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserNewDeviceFound(bool f)
{
  auto *cmd = new SetUserNewDeviceFoundCommand(c, f);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserTrainingWheels(bool w)
{
  auto *cmd = new SetUserTrainingWheelsCommand(c, w);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserLocale(const Enum<Locale> &l)
{
  auto *cmd = new SetUserLocaleCommand(c, l);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserTimeFormat(const Enum<TimeFormat> &f)
{
  auto *cmd = new SetUserTimeFormatCommand(c, f);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setUserUnknownProperty(
    const data::item::UnknownElement &value)
{
  auto *cmd = new SetUserUnknownPropertyCommand(c, value);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setControllerId(uint32_t id)
{
  auto *cmd = new SetControllerIdCommand(c, id);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setControllerMetadata(const QString &type,
    const QString &mnf, const QString &model, const QString &label)
{
  auto *cmd = new SetControllerMetadataCommand(c, type, mnf, model, label);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setControllerUnknownProperty(
    const data::item::UnknownElement &value)
{
  auto *cmd = new SetUserUnknownPropertyCommand(c, value);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::addDeviceCommand(int pos, uint32_t id)
{
  auto *cmd = new AddDeviceCommand(c, id, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device pos %1 already exists, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return true;
}

bool CmdCatalogue::addDeviceCommand(int pos, uint32_t *id)
{
  auto *cmd = new AddDeviceCommand(c, pos);
  if (id != nullptr) {
    *id = cmd->getUid();
  }
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::removeDeviceCommand(int pos)
{
  auto *cmd = new RemoveDeviceCommand(c, pos);
  auto ret = cmd->valid();
  if (ret == true) {
    connectCommand(cmd);
    undo.push(cmd);
  } else {
    emit writeLog(LogLevel::Warning,
        tr("modify: device pos %1 doesn't exist, dropped").arg(pos),
        ContentType::PlainText);
    delete cmd;
  }
  return ret;
}

bool CmdCatalogue::setDeviceMetadata(const Enum<DeviceType> &type,
    const QString &mnf, const QString &model, const QString &label,
    uint32_t pos)
{
  auto *cmd = new SetDeviceMetadataCommand(c, type, mnf, model, label, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceMnf(const QString &v, uint32_t pos)
{
  auto *cmd = new SetDeviceMnfCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceModel(const QString &v, uint32_t pos)
{
  auto *cmd = new SetDeviceModelCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceLabel(const QString &v, uint32_t pos)
{
  auto *cmd = new SetDeviceLabelCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceManualPower(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceManualPowerCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceAlwaysOn(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceAlwaysOnCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceAutoPower(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceAutoPowerCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceAudioSwitch(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceAudioSwitchCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceDimmer(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceDimmerCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceHasBands(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceHasBandsCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceHasPresets(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceHasPresetsCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceIsNewDevice(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceIsNewDeviceCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceIsDisplayDevice(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceIsDisplayDeviceCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceMenuOnDevice(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceMenuOnDeviceCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceOnScreenGuide(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceOnScreenGuideCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceRecordMediaFixedDisc(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceRecordMediaFixedDiscCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceRecordMediaRemovableVideotape(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceRecordMediaRemovableVideotapeCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceRevertInput(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceRevertInputCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceScart(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceScartCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceVideoSwitch(bool v, uint32_t pos)
{
  auto *cmd = new SetDeviceVideoSwitchCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceNumDiscs(int32_t v, uint32_t pos)
{
  auto *cmd = new SetDeviceNumDiscsCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceNumLights(int32_t v, uint32_t pos)
{
  auto *cmd = new SetDeviceNumLightsCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceType(const Enum<DeviceType> &v, uint32_t pos)
{
  auto *cmd = new SetDeviceTypeCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDevicePvrType(const Enum<PvrType> &v, uint32_t pos)
{
  auto *cmd = new SetDevicePvrTypeCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceTunerInput(const Enum<TunerInput> &v, uint32_t pos)
{
  auto *cmd = new SetDeviceTunerInputCommand(c, v, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

bool CmdCatalogue::setDeviceUnknownProperty(
    const data::item::UnknownElement &value, uint32_t pos)
{
  auto *cmd = new SetDeviceUnknownPropertyCommand(c, value, pos);
  connectCommand(cmd);
  undo.push(cmd);
  return true;
}

void CmdCatalogue::connectCommand(BaseCommand *cmd)
{
  //connect signals. not all commands actually use all signals!

  // @formatter:off
  connect(cmd, &BaseCommand::writeLog, this, &CmdCatalogue::writeLog);
  connect(cmd, &BaseCommand::writeMsg, this, &CmdCatalogue::writeMsg);
  connect(cmd, &BaseCommand::deviceChanged, this, &CmdCatalogue::deviceChanged);
  connect(cmd, &BaseCommand::deviceAboutToBeAdded, this, &CmdCatalogue::deviceAboutToBeAdded);
  connect(cmd, &BaseCommand::deviceAdded, this, &CmdCatalogue::deviceAdded);
  connect(cmd, &BaseCommand::deviceAboutToBeRemoved, this, &CmdCatalogue::deviceAboutToBeRemoved);
  connect(cmd, &BaseCommand::deviceRemoved, this, &CmdCatalogue::deviceRemoved);
  connect(cmd, &BaseCommand::activityChanged, this, &CmdCatalogue::activityChanged);
  connect(cmd, &BaseCommand::activityAboutToBeAdded, this, &CmdCatalogue::activityAboutToBeAdded);
  connect(cmd, &BaseCommand::activityAdded, this, &CmdCatalogue::activityAdded);
  connect(cmd, &BaseCommand::activityAboutToBeRemoved, this, &CmdCatalogue::activityAboutToBeRemoved);
  connect(cmd, &BaseCommand::activityRemoved, this, &CmdCatalogue::activityRemoved);
  connect(cmd, &BaseCommand::dirtyChanged, this, &CmdCatalogue::dirtyChanged);
// @formatter:on
}

}
}

