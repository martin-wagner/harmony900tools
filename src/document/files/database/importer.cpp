// SPDX-License-Identifier: LGPL-2.1-or-later

#include <boost/algorithm/string.hpp>
#include <fstream>

#include <nlohmann/json.hpp>

#include "lib/qtHelpers.h"
#include "lib/uid.h"
#include "importer.h"
#include "keyCode.h"
#include "document/data/items/device.h"
#include "document/data/items/jsonSerialise.h"

using namespace nlohmann;
using namespace std;

namespace document
{
namespace files
{

Database::Database(QObject *parent) :
    QObject(parent)
{
  clear();
}

bool Database::setDatabase(const DatabasePaths &paths)
{
  json j;

  auto keyLc = strq(keyCatalogueLc);
  auto keyMnf = strq(keyCatalogueMnf);
  auto keyMnfName = strq(keyCatalogueMnfName);
  auto keyDev = strq(keyCatalogueDev);
  auto keyDevName = strq(keyCatalogueDevName);
  auto keyDevType = strq(keyCatalogueDevType);
  auto keyDevId = strq(keyCatalogueDevId);
  auto keyDevLine = strq(keyCatalogueDevLine);

  if (!paths.valid()) {
    clear();
    return false;
  }
  if (paths == d) {
    //no change, keep current data
    return true;
  }
  clear();
  d = paths;

  auto size = QFile(d.catalogue).size();
  if (size == 0) {
    emit writeLog(LogLevel::Error, tr("load: %1 is empty").arg(d.catalogue),
        ContentType::PlainText);
    clear();
    return false;
  }

#ifdef _WIN32
  ifstream f(d.catalogue.toStdWString().c_str(), ios::binary);
#else
  ifstream f(d.catalogue.toUtf8(), ios::binary);
#endif
  if (!f.is_open()) {
    emit writeLog(LogLevel::Error,
        tr("load: failed to open %1").arg(d.catalogue), ContentType::PlainText);
    clear();
    return false;
  }
  try {
    j = json::parse(f);
  } catch (const json::parse_error &e) {
    emit writeLog(LogLevel::Error,
        tr("load: Json parse error at byte %1: %2").arg(e.byte).arg(
            QString(e.what())), ContentType::PlainText);
    clear();
    return false;
  } catch (const json::exception &e) {
    emit writeLog(LogLevel::Error,
        tr("load: Json error [%1]: %2").arg(e.id).arg(QString(e.what())),
        ContentType::PlainText);
    clear();
    return false;
  }

  auto version = j.at(strq(keyCatalogueVersion)).get<uint32_t>();
  if (version != catalogueVersion) {
    emit writeLog(LogLevel::Error,
        tr("load %1: unsupported version %2").arg(d.catalogue).arg(version),
        ContentType::PlainText);
    clear();
    return false;

  }
  if ((!j.contains(keyLc)) || !j.at(keyLc).is_number_unsigned()) {
    emit writeLog(LogLevel::Error,
        tr("load %1: no Device entries").arg(d.catalogue),
        ContentType::PlainText);
    clear();
    return false;
  }
  deviceCount = j.at(keyLc).get<uint64_t>();

  if (!j.contains(keyMnf)) {
    emit writeLog(LogLevel::Error,
        tr("load %1: no Manufacturer entries").arg(d.catalogue),
        ContentType::PlainText);
    clear();
    return false;
  }

  if (!j.at(keyMnf).is_array()) {
    emit writeLog(LogLevel::Error,
        tr("load %1: Manufacturers is not an array").arg(d.catalogue),
        ContentType::PlainText);
    clear();
    return false;
  }
  for (const auto &mnfEntry : j.at(keyMnf)) {
    Manufacturer manufacturer;

    if (!mnfEntry.contains(keyMnfName)
        || !mnfEntry.at(keyMnfName).is_string()) {
      emit writeLog(LogLevel::Notice,
          tr("load %1: Manufacturer entry missing Name").arg(d.catalogue),
          ContentType::PlainText);
      continue;
    }
    manufacturer.name = QString::fromStdString(
        mnfEntry.at(keyMnfName).get<string>());
    QString mnfKey = normaliseKey(manufacturer.name);

    if (!mnfEntry.contains(keyDev) || !mnfEntry.at(keyDev).is_array()) {
      emit writeLog(LogLevel::Notice,
          tr("load %1: Manufacturer %2 missing Devices").arg(d.catalogue).arg(
              manufacturer.name), ContentType::PlainText);
      continue;
    }

    for (const auto &devEntry : mnfEntry.at(keyDev)) {
      Device device;

      if (!devEntry.contains(keyDevName)
          || !devEntry.at(keyDevName).is_string()) {
        emit writeLog(LogLevel::Notice,
            tr("load %1: Device entry missing Name (%2)").arg(d.catalogue).arg(
                manufacturer.name), ContentType::PlainText);
        continue;
      }
      if (!devEntry.contains(keyDevType)
          || !devEntry.at(keyDevType).is_number_unsigned()) {
        emit writeLog(LogLevel::Notice,
            tr("load %1: Device entry missing Type (%2)").arg(d.catalogue).arg(
                manufacturer.name), ContentType::PlainText);
        continue;
      }
      if (!devEntry.contains(keyDevId)
          || !devEntry.at(keyDevId).is_number_unsigned()) {
        emit writeLog(LogLevel::Notice,
            tr("load %1: Device entry missing Id (%2)").arg(d.catalogue).arg(
                manufacturer.name), ContentType::PlainText);
        continue;
      }
      if (!devEntry.contains(keyDevLine)
          || !devEntry.at(keyDevLine).is_number_unsigned()) {
        emit writeLog(LogLevel::Notice,
            tr("load %1: Device entry missing Line (%2)").arg(d.catalogue).arg(
                manufacturer.name), ContentType::PlainText);
        continue;
      }

      device.name = QString::fromStdString(
          devEntry.at(keyDevName).get<string>());
      auto typeValue = devEntry.at(keyDevType).get<uint32_t>();

      device.type = data::Enum<data::DatabaseDeviceType>(
          static_cast<data::DatabaseDeviceType>(typeValue));
      device.id = devEntry.at(keyDevId).get<uint64_t>();
      device.line = devEntry.at(keyDevLine).get<uint64_t>();

      auto devKey = normaliseKey(device.name);
      manufacturer.devices.emplace(devKey, device);
    }

    db.emplace(mnfKey, move(manufacturer));
  }

  return true;
}

QStringList Database::findManufacturers(const QString &substr) const
{
  QStringList result;
  QString key = normaliseKey(substr);

  for (const auto &entry : db) {
    if (entry.first.contains(key)) {
      result.append(entry.second.name);
    }
  }

  return result;
}

QStringList Database::findDevices(const QString &substr,
    QString *manufacturerName) const
{
  QStringList result;
  QString key = normaliseKey(substr);

  for (const auto &mnfEntry : db) {
    for (const auto &devEntry : mnfEntry.second.devices) {
      if (devEntry.first.contains(key)) {
        result.append(devEntry.second.name);
        if (manufacturerName != nullptr) {
          *manufacturerName = mnfEntry.second.name;
        }
      }
    }
  }

  return result;
}

QStringList Database::findDevices(const QString &manufacturerName,
    const QString &substr) const
{
  QStringList result;

  auto mnfIt = db.find(normaliseKey(manufacturerName));
  if (mnfIt == db.end()) {
    return result;
  }

  QString key = normaliseKey(substr);

  for (const auto &entry : mnfIt->second.devices) {
    if (entry.first.contains(key)) {
      result.append(entry.second.name);
    }
  }

  return result;
}

Database::Device Database::getDeviceInfo(const QString &manufacturerName,
    const QString &deviceName) const
{
  auto mnf = normaliseKey(manufacturerName);
  auto dev = normaliseKey(deviceName);

  try {
    auto &data = db.at(mnf).devices.at(dev);
    return data;
  } catch (out_of_range&) {
    return Device();
  }
}

bool document::files::Database::importDevice(const QString &manufacturerName,
    const QString &deviceName, document::Config &config,
    bool tryConvertProtocol) const
{
  auto source = getDeviceInfo(manufacturerName, deviceName);
  if (source.name.isEmpty()) {
    return false;
  }
  return importDevice(source, config, tryConvertProtocol);
}

bool document::files::Database::importDevice(const Device &source,
    document::Config &config, bool tryConvertProtocol) const
{
  json j;
  string line;

  if (d.devices.isEmpty()) {
    return false;
  }

  auto size = QFile(d.devices).size();
  if (size == 0) {
    emit writeLog(LogLevel::Error, tr("load: %1 is empty").arg(d.devices),
        ContentType::PlainText);
    return false;
  }

#ifdef _WIN32
  ifstream f(d.devices.toStdWString().c_str(), ios::binary);
#else
  ifstream f(d.devices.toUtf8(), ios::binary);
#endif
  if (!f.is_open()) {
    emit writeLog(LogLevel::Error, tr("load: failed to open %1").arg(d.devices),
        ContentType::PlainText);
    return false;
  }

  for (uint64_t i = 0; i <= source.line; i++) {
    if (!getline(f, line)) {
      emit writeLog(LogLevel::Error,
          tr("load %1: line %2 not found").arg(d.devices).arg(source.line),
          ContentType::PlainText);
      return false;
    }
  }

  try {
    j = json::parse(line);
  } catch (const json::parse_error &e) {
    emit writeLog(LogLevel::Error,
        tr("load %1: Json parse error at line %2, byte %3: %4").arg(d.devices).arg(
            source.line).arg(e.byte).arg(QString(e.what())),
        ContentType::PlainText);
    return false;
  } catch (const json::exception &e) {
    emit writeLog(LogLevel::Error,
        tr("load %1: Json error at line %2 [%3]: %4").arg(d.devices).arg(
            source.line).arg(e.id).arg(QString(e.what())),
        ContentType::PlainText);
    return false;
  }

  if ((!j.contains("id")) || !j.at("id").is_number_unsigned()) {
    emit writeLog(LogLevel::Error,
        tr("load %1: line %2 has no id").arg(d.devices).arg(source.line),
        ContentType::PlainText);
    return false;
  }

  auto id = j.at("id").get<uint64_t>();
  if (id != source.id) {
    emit writeLog(LogLevel::Error,
        tr("load %1: line %2 id mismatch (expected %3, got %4)").arg(d.devices).arg(
            source.line).arg(source.id).arg(id), ContentType::PlainText);
    return false;
  }

  try {
    return importDevice(j, config, tryConvertProtocol);
  } catch (const json::exception &e) {
    emit writeLog(LogLevel::Error,
        tr("load %1: Json error at line %2 [%3]: %4").arg(d.devices).arg(
            source.line).arg(e.id).arg(QString(e.what())),
        ContentType::PlainText);
    return false;
  }
}

void Database::clear()
{
  db.clear();
  deviceCount = 0;
  d = DatabasePaths();
}

QString Database::normaliseKey(const QString &s)
{
  QString result;
  result.reserve(s.size());

  for (const QChar &c : s) {
    if (c.isLetterOrNumber() && (c.unicode() < 128)) {
      result.append(c.toLower());
    }
  }

  return result;
}

bool Database::importDevice(const json &j, document::Config &config,
    bool tryConvertProtocol) const
{
  bool ret = true;

  bool hasChannelTuningFeature = false;
  bool hasInputFeature = false;
  bool hasPowerFeature = false;
  bool hasInternalStateFeature = false;
  bool hasOutputFeature = false;
  detectDeviceFeatures(j, hasChannelTuningFeature, hasInputFeature,
      hasPowerFeature, hasInternalStateFeature, hasOutputFeature);

  data::item::Device d(lib::UidGenerator::getInstance().generate()); //don't use devices own id, create instance id
  auto &jd = j.at("device");

  //description
  data::serialiser::fromJson(jd, "DeviceType", d.type);
  data::serialiser::fromJson(jd, "Manufacturer", d.mnf);
  data::serialiser::fromJson(jd, "Model", d.model);
  data::serialiser::fromJson(jd, "Name", d.label);

  //properties
  if (hasPowerFeature) {
    auto jp = getFeature(j, "ChannelTuningFeature");
    data::serialiser::fromJson(jd, "IsPowerAlwaysOn", d.manualPower);
    data::serialiser::fromJson(jd, "IsPoweredOnBetweenActivities", d.alwaysOn);
  }
  auto display = hasCapability(jd, Capabilities::IS_DISPLAY);
  if (display) {
    d.isDisplayDevice.set(true).setIncluded(data::Used::YES);
  }
  auto tuner = hasCapability(jd, Capabilities::HAS_TUNER_INPUT);
  if (tuner) {
    d.tunerInput.set(data::TunerInput::Tuner).setIncluded(data::Used::YES);
  }
  data::serialiser::fromJson(jd, "IsScartCableSupported", d.scart);
  //fixme more?

  if (j.contains("commands") && j.at("commands").is_array()) {
    ret &= commands(j, d, config, tryConvertProtocol);
  }

  //todo state machines

  //todo numpad

  //delays
  data::serialiser::fromJson(jd, "InterDeviceDelay",
      d.getIrCommands().pressPreSilenceMs);
  data::serialiser::fromJson(jd, "InterKeyDelay",
      d.getIrCommands().pressInterKeyMs);
  data::serialiser::fromJson(jd, "HoldInterDeviceDelay",
      d.getIrCommands().holdPreSilenceMs);
  data::serialiser::fromJson(jd, "HoldInterKeyDelay",
      d.getIrCommands().holdInterKeyMs);

  return true;
}

bool document::files::Database::commands(const nlohmann::json &j,
    data::item::Device &device, document::Config &config,
    bool tryConvertProtocol) const
{
  bool ret = true;

  for (const auto &cmd : j.at("commands")) {
    if (!cmd.contains("CommandTypeId")) { //hard button enum item, contains buttons we don't have!
      continue;
    }
    auto res = command(cmd, device, config, tryConvertProtocol);
    if (!res) {
      ret = false;
      continue;
    }
    //add button only if parsing / creating command was successful
    auto cmdTypeStr = cmd.at("CommandTypeId").get<string>();
    auto cmdTypeEnum = data::Enum<data::HardButtons>(cmdTypeStr);
    if (cmdTypeEnum.getValue() == data::HardButtons::Unknown) {
      ret &= softButton(cmd, cmdTypeStr, device);
    } else {
      ret &= hardButton(cmd, cmdTypeStr, device);
    }
  }
  return ret;
}

bool document::files::Database::command(const nlohmann::json &j,
    data::item::Device &device, document::Config &config,
    bool tryConvertProtocol) const
{
  data::item::ProtoCommand cmd;
  KeyCode keyCode;

  if (j.contains("Raw") && !j.at("Raw").is_null()) {
    //fixme used by 19 of approx. 270000 devices. not worth the effort.
    emit writeLog(LogLevel::Warning,
        tr("inport %1: RAW commands not supported").arg(
            qstr(device.label.get())), ContentType::PlainText);
    return false;
  }

  data::serialiser::fromJson(j, "Name", cmd.name);
  if (cmd.name.get().empty()) {
    return false;
  }
  if (boost::iequals(cmd.name.get(), "Unknown")) {
    return false;
  }
  if (!j.contains("ProtocolId") || !j.at("ProtocolId").is_number_unsigned()
      || !j.contains("KeyCode") || !j.at("KeyCode").is_string()) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO data missing/invalid").arg(
            qstr(device.label.get())), ContentType::PlainText);
    return false;
  }
  auto protocolId = j.at("ProtocolId").get<uint32_t>();
  //todo  createIrProto();

  auto keyCodeStr = j.at("KeyCode").get<string>();
  auto res = KeyCode::parse(qstr(keyCodeStr), keyCode);
  if (res == false) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO KeyCode parser error").arg(
            qstr(device.label.get())), ContentType::PlainText);
    return false;
  }
  //fixme the following verifies that we actually can encode the data into
  //irProt.bin / userconfig.xml code fields. this needs a sample for each and every case.
  //without that, best we _might_ do is serialise and store as raw command
  if ((keyCode.getGroup() != "G")
  //|| (keyCode.getTrailer() != 3) //fixme not always 3 -- just ignore for now.
  ) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO header/trailer not supported").arg(
            qstr(device.label.get())), ContentType::PlainText);
    return false;
  }
  if (keyCode.isQuadBitType()) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO quad type not supported").arg(
            qstr(device.label.get())), ContentType::PlainText);
    return false;
  }
  if (keyCode.isHexBitType()) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO hex type not supported").arg(
            qstr(device.label.get())), ContentType::PlainText);
    return false;
  }

  //todo wie das machen...
  if ((keyCode.getItemType(0) != KeyCode::ItemType::Empty)
      && (keyCode.getItemType(0) != KeyCode::ItemType::Number)) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO item0 %2 not supported").arg(
            qstr(device.label.get())).arg(keyCode.getItem(0).toString()),
        ContentType::PlainText);
    return false;
  }
  if ((keyCode.getItemType(1) != KeyCode::ItemType::Number)
      && (keyCode.getItemType(1) != KeyCode::ItemType::Array)
      && (keyCode.getItemType(1) != KeyCode::ItemType::Text)) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO item1 %2 not supported").arg(
            qstr(device.label.get())).arg(keyCode.getItem(1).toString()),
        ContentType::PlainText);
    return false;
  }
  if ((keyCode.getItemType(1) != KeyCode::ItemType::Number)
      && (keyCode.getItemType(1) != KeyCode::ItemType::Array)
      && (keyCode.getItemType(1) != KeyCode::ItemType::Text)) {
    emit writeLog(LogLevel::Warning,
        tr("inport %1: PROTO item1 %2 not supported").arg(
            qstr(device.label.get())).arg(keyCode.getItem(1).toString()),
        ContentType::PlainText);
    return false;
  }

  //end fixme

  return false;
}

bool document::files::Database::softButton(const nlohmann::json &j,
    const string &type, data::item::Device &device) const
{
  data::item::Button button;

  data::serialiser::fromJson(j, "Name", button.action);
  //validity check in "command()"
  button.name = button.action.get(); //both the same
  button.name.setIncluded(data::Used::YES);
  button.position.set(device.getSoftButtons().size() + 1).setIncluded(
      data::Used::YES);
  device.getSoftButtons().push_back(button);
  return true;
}

bool document::files::Database::hardButton(const nlohmann::json &j,
    const string &type, data::item::Device &device) const
{
  data::item::Button button;

  data::serialiser::fromJson(j, "Name", button.action);
  if (button.action.get().empty()) {
    button.action.set(type);
  }
  button.name.set(type).setIncluded(data::Used::YES);
  device.getHardButtons().push_back(button);
  return true;
}

void Database::detectDeviceFeatures(const json &j,
    bool &hasChannelTuningFeature, bool &hasInputFeature, bool &hasPowerFeature,
    bool &hasInternalStateFeature, bool &hasOutputFeature) const
{
  hasChannelTuningFeature = false;
  hasInputFeature = false;
  hasPowerFeature = false;
  hasInternalStateFeature = false;
  hasOutputFeature = false;

  if (!j.contains("features") || !j.at("features").is_array()) {
    return;
  }

  for (const auto &feature : j.at("features")) {
    if (!feature.contains("__type")) {
      continue;
    }

    const string type = feature.at("__type").get<string>();

    if (type == "ChannelTuningFeature") {
      hasChannelTuningFeature = true;
    } else if (type == "InputFeature") {
      hasInputFeature = true;
    } else if (type == "PowerFeature") {
      hasPowerFeature = true;
    } else if (type == "InternalStateFeature") {
      hasInternalStateFeature = true;
    } else if (type == "OutputFeature") {
      hasOutputFeature = true;
    }
  }
}

const json& document::files::Database::Database::getFeature(const json &j,
    const string &type) const
{
  if (!j.contains("features") || !j.at("features").is_array()) {
    throw runtime_error("features array missing");
  }

  for (const auto &feature : j.at("features")) {
    if (feature.contains("__type")
        && feature.at("__type").get<string>() == type) {
      return feature;
    }
  }

  throw runtime_error("feature not found: " + type);
}

bool document::files::Database::hasCapability(const nlohmann::json &j,
    Capabilities capability) const
{
  for (const auto &capabilities : j.at("DeviceCapabilitiesWithPriority")) {
    if (!capabilities.contains("DeviceCapability")) {
      continue;
    }

    auto value = capabilities.at("DeviceCapability").get<int>();
    if (value == static_cast<int>(capability)) {
      return true;
    }
  }
  return false;
}

data::DeviceType Database::mapDatabaseDeviceType(
    data::DatabaseDeviceType source)
{
  //lookup table
  switch (source) {
    case data::DatabaseDeviceType::Television:
      return data::DeviceType::Television;
    case data::DatabaseDeviceType::VCR:
      return data::DeviceType::Vcr;
    case data::DatabaseDeviceType::CDPlayer:
      return data::DeviceType::Cd;
    case data::DatabaseDeviceType::DVD:
      return data::DeviceType::DvdCd;
    case data::DatabaseDeviceType::StereoReceiver:
      return data::DeviceType::Receiver;
    case data::DatabaseDeviceType::TapeDeck:
      return data::DeviceType::Cassette;
    case data::DatabaseDeviceType::Projector:
      return data::DeviceType::Projector;
    case data::DatabaseDeviceType::Computer:
      return data::DeviceType::Computer;
    case data::DatabaseDeviceType::GameConsole:
      return data::DeviceType::GameConsole;
    case data::DatabaseDeviceType::Laptop:
      return data::DeviceType::Laptop;
    case data::DatabaseDeviceType::CDJukebox:
      return data::DeviceType::CdJukebox;
    case data::DatabaseDeviceType::DigitalSetTopBox:
      return data::DeviceType::SetTopBox;
    case data::DatabaseDeviceType::Monitor:
      return data::DeviceType::Monitor;
    case data::DatabaseDeviceType::CableBox:
      return data::DeviceType::CableBox;
    case data::DatabaseDeviceType::TVVCR:
      return data::DeviceType::TvVcr;
    case data::DatabaseDeviceType::Satellite:
      return data::DeviceType::Satellite;
    case data::DatabaseDeviceType::PVR:
      return data::DeviceType::Pvr;
    case data::DatabaseDeviceType::Amplifier:
      return data::DeviceType::Amplifier;
    case data::DatabaseDeviceType::DAT:
      return data::DeviceType::Dat;
    case data::DatabaseDeviceType::MiniSystemCDRadioCassette:
      return data::DeviceType::CdRadioCassette;
    case data::DatabaseDeviceType::MiniSystemDvdCDRadio:
      return data::DeviceType::DvdCdRadio;
    case data::DatabaseDeviceType::GameConsoleWithDvd:
      return data::DeviceType::DvdCdGame;
    case data::DatabaseDeviceType::LightController:
      return data::DeviceType::Light;
    case data::DatabaseDeviceType::DVDVCR:
      return data::DeviceType::DvdVcr;
    case data::DatabaseDeviceType::LaserdiscPlayer:
      return data::DeviceType::Laserdisc;
    case data::DatabaseDeviceType::MinidiscPlayer:
      return data::DeviceType::Minidisc;
    case data::DatabaseDeviceType::AudioVideoSwitch:
      return data::DeviceType::AudioVideoSwitch;
    case data::DatabaseDeviceType::MiniSystemDvdVcrRadio:
      return data::DeviceType::DvdVcrReceiver;
    case data::DatabaseDeviceType::DigitalMusicServer:
      return data::DeviceType::MusicServer;
    case data::DatabaseDeviceType::DVDRecorder:
      return data::DeviceType::DvdRecorder;
    case data::DatabaseDeviceType::MediaCenterPC:
      return data::DeviceType::MediaCenterPC;
    case data::DatabaseDeviceType::RadioTuner:
      return data::DeviceType::RadioTuner;
    case data::DatabaseDeviceType::TVDVD:
      return data::DeviceType::TvDvd;
    case data::DatabaseDeviceType::TVDVDVCR:
      return data::DeviceType::TvDvdVcr;
    case data::DatabaseDeviceType::HomeAppliance:
      return data::DeviceType::HomeAppliance;
    case data::DatabaseDeviceType::ClimateControl:
      return data::DeviceType::ClimateControl;
    case data::DatabaseDeviceType::DVDRVCR:
      return data::DeviceType::DvdrVcr;
    case data::DatabaseDeviceType::TVHDD:
      return data::DeviceType::TvHdd;
    case data::DatabaseDeviceType::TVCamera:
      return data::DeviceType::Television;
    case data::DatabaseDeviceType::AppleTV:
      return data::DeviceType::SetTopBox;
    case data::DatabaseDeviceType::Roku:
      return data::DeviceType::SetTopBox;
    case data::DatabaseDeviceType::AutomationGateway:
      return data::DeviceType::HomeAppliance;
    case data::DatabaseDeviceType::Controller:
      return data::DeviceType::Computer;
    case data::DatabaseDeviceType::Thermostat:
      return data::DeviceType::ClimateControl;
    case data::DatabaseDeviceType::Camera:
      return data::DeviceType::Computer;
    case data::DatabaseDeviceType::MediaPlayer:
      return data::DeviceType::Computer;
    case data::DatabaseDeviceType::Plug:
      return data::DeviceType::Computer;
    case data::DatabaseDeviceType::Default:
      return data::DeviceType::Computer;
    case data::DatabaseDeviceType::Unknown:
      return data::DeviceType::Computer;
    default:
      return data::DeviceType::Computer;
  }
}

}
}
