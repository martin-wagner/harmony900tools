// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QFile>
#include <QFutureWatcher>
#include <QObject>
#include <QStringList>

#include <atomic>
#include <functional>

#include "document/data/enum.h"
#include "ui/logViewer.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace document
{
namespace files
{

enum class StepState
{
  Idle, Running, Done, Failed, Skipped
};

enum class DatabaseCatalogueFileType
{
  Raw, Devices, Protocols

};

struct SourceSpec
{
    DatabaseCatalogueFileType type;
    QString fileName;
    QString location;   ///< url or local path
    bool isLocal = false;
};

/**
 * Everything the controller needs to start an import.
 * The order of sources matches the rows in the ui: the data files first,
 * the checksum file is stored separately.
 */
struct ImporterConfig
{
    QString targetDir;
    QList<SourceSpec> sources;
};

/**
 * Database files
 *
 * Imported / processed files, ready for further use.
 *
 * - catalogue -- json containing mnf/devicename/deviceid list for fast search
 * - raw -- one line per device id in json format, short form, mostly subset of "devices"
 * - devices -- one line per device id in json format, full dataset to import one device
 * - protocols -- json containing all protocols, corresponds to "devices"
 */
struct DatabasePaths
{
    //individual filenames. full path
    QString catalogue;
    QString raw;
    QString devices;
    QString protocols;
};

//forward
struct NameLess;
struct DeviceEntry;
using Catalogue = std::map<QString, std::vector<DeviceEntry>, NameLess>;

/**
 * Runs the import: download (or use the local file), extract,
 * build the index. Files are processed one after another.
 *
 * Lives in the ui thread. Downloading is asynchronous (QNetworkAccessManager),
 * extracting and indexing run in worker threads (QtConcurrent).
 *
 * Source indices match ImporterConfig::sources. Index -1 means the index step.
 *
 * produces DatabaseFiles as output
 */
class ImportDatabase: public QObject
{
  Q_OBJECT

  public:
    explicit ImportDatabase(QObject *parent = nullptr);
    ~ImportDatabase() override;

    inline static const QString catalogueFileName = "catalogue.json";
    inline static const QString keyCatalogueVersion = "Version";
    static constexpr uint32_t catalogueVersion = 1;
    inline static const QString keyCatalogueLc = "LineCount";
    inline static const QString keyCatalogueMnf = "Manufacturers";
    inline static const QString keyCatalogueMnfName = "Name";
    inline static const QString keyCatalogueDev = "Devices";
    inline static const QString keyCatalogueDevName = "Name";
    inline static const QString keyCatalogueDevType = "Type";
    inline static const QString keyCatalogueDevId = "Id";
    inline static const QString keyCatalogueDevLine = "Line";

    bool isRunning() const;

  signals:
    void sourceStateChanged(int index, StepState state, const QString &message);
    void sourceProgressChanged(int index, uint64_t done, uint64_t total);
    void indexStateChanged(StepState state, const QString &message);
    void indexProgressChanged(uint64_t done, uint64_t total);
    void logMessage(const QString &line);
    void finished(bool success, const DatabasePaths paths);

  public slots:
    void start(const ImporterConfig &newConfig);
    void cancel();

  private slots:
    void onDownloadReadyRead();
    void onDownloadProgress(uint64_t received, uint64_t total);
    void onDownloadFinished();
    void onExtractFinished();
    void onIndexFinished();

  private:
    void processSource(int index);
    void startDownload(int index, const QString &location, const QString &targetPath);
    void extractFile(int index);
    void finishSource(int index);
    void buildIndex();
    void fail(int index, const QString &message);
    void finish(bool success);
    static QByteArray jsonString(const QString &value);
    static bool writeIndex(const QString &indexPath, const QString &sourceName, uint64_t lineCount, const Catalogue &catalogue, QString &errorText);
    static QString getExtractedName(const QString &fileName);
    QString getExtractedPath(int index) const;

    /** Progress callback for worker threads. Throttled, returns false when cancelled. */
    std::function<bool(uint64_t, uint64_t)> createReporter(int index);

    bool createCatalogueIndex(const DatabasePaths &paths, const std::function<bool(uint64_t, uint64_t)> &progress);

    ImporterConfig config;
    bool running = false;
    std::atomic<bool> cancelRequested { false };
    int currentIndex = -1;

    QStringList compressedPaths;
    DatabasePaths extractedPaths;

    QNetworkAccessManager *network = nullptr;
    QNetworkReply *reply = nullptr;
    QFile downloadFile;
    QString downloadTargetPath;
    bool downloadWriteFailed = false;

    QFutureWatcher<bool> extractWatcher;
    QFutureWatcher<bool> indexWatcher;
};

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

    bool setDatabase(const DatabasePaths &paths);

    uint64_t getDeviceCount() { return deviceCount; };

    QStringList findManufacturers(const QString &substr) const;
    QStringList findDevices(const QString &substr) const;
    QStringList findDevices(const QString &manufacturerName, const QString &substr) const;

    /** get info to exactly one device. if not found device.name is empty */
    Device getDeviceInfo(const QString &manufacturerName, const QString &deviceName);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

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

  private:
    void clear();
    static QString normaliseKey(const QString &s);

};

}
}
