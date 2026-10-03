// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "database.h"
#include "bin/irProto/file.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace document
{
namespace files
{

/** reads the catalogue and pulls device/protocol from the device/protocol jsons */
class Database: public QObject
{
  Q_OBJECT

  public:
    struct Device {
      QString name;
      data::Enum<data::DatabaseDeviceType> type = data::DatabaseDeviceType::Unknown;
      uint64_t id;
      uint64_t line;
    };

  public:
    Database();

    /** parse files */
    bool setDatabase(const DatabasePaths &paths);

    /** how many devices the database contains */
    uint64_t getDeviceCount() { return deviceCount; };

    /**
     * find a manufacturer.
     *
     * substr is stripped of everything not a...z/0...9
     */
    QStringList findManufacturers(const QString &substr) const;

    /**
     * find a device.
     *
     * substr is stripped of everything not a...z/0...9
     * manufacturerName only valid when one match is found
     */
    QStringList findDevices(const QString &substr, QString *manufacturerName = nullptr) const;

    /**
     * find a device.
     *
     * manufacturer must match the list. then substr is stripped of everything
     * not a...z/0...9.
     */
    QStringList findDevices(const QString &manufacturerName, const QString &substr) const;

    /** get info to exactly one device. if not found ret.device.name is empty */
    Device getDeviceInfo(const QString &manufacturerName, const QString &deviceName) const;

    /** create data for importing exactly one device
     * @param tryConvertProtocol tries to convert the proprietary protocols
     * to one of ours, so edting will be possible */
    bool importDevice(const QString &manufacturerName, const QString &deviceName, document::Config &config, bool tryConvertProtocol = true) const;

    /** create data for importing for exactly one device
     * @param tryConvertProtocol tries to convert the proprietary protocols
     * to one of ours, so edting will be possible */
    bool importDevice(const Device &source, document::Config &config, bool tryConvertProtocol = true) const;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType) const;
    void writeMsg(const QString &message) const;

  protected:

    struct Manufacturer {
      QString name;
      std::unordered_map<QString, Device> devices;
    };

  protected:
    DatabasePaths d;

    uint64_t deviceCount;
    //lowercase/stripped mnf name / devices
    std::unordered_map<QString, Manufacturer> db;

    //ProtocolId, Protocol
    mutable std::unordered_map<uint32_t, binary::irProto::IrProto> irProtoCache;

  private:
    //ai did some guessing...
    enum class Capabilities
    {
      IS_MANUAL_POWER = 1,    //maybe?
      IS_DISPLAY = 2,
      HAS_TUNER_INPUT = 3,
      IS_SOURCE = 5,          //maybe?
      HAS_MEDIA = 6,          //maybe?
      IS_SINK = 8,            //maybe?
    };

  private:
    void clear();
    static QString normaliseKey(const QString &s);
    bool importDevice(const nlohmann::json &j, document::Config &config, bool tryConvertProtocol) const;
    bool commands(const nlohmann::json &j, data::item::Device &device, document::Config &config, bool tryConvertProtocol) const;
    bool command(const nlohmann::json &j, data::item::Device &device, document::Config &config, bool tryConvertProtocol) const;
    bool softButton(const nlohmann::json &j, const std::string &type, data::item::Device &device) const;
    bool hardButton(const nlohmann::json &j, const std::string &type, data::item::Device &device) const;
    void detectDeviceFeatures(const nlohmann::json &j, bool &hasChannelTuningFeature, bool &hasInputFeature, bool &hasPowerFeature, bool &hasInternalStateFeature, bool &hasOutputFeature) const;
    const nlohmann::json &getFeature(const nlohmann::json &j, const std::string &type) const;
    bool hasCapability(const nlohmann::json &j, Capabilities capability) const;
    static data::DeviceType mapDatabaseDeviceType(data::DatabaseDeviceType source);


};

}
}
