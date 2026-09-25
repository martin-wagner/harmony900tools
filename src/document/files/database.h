// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QFile>
#include <QFutureWatcher>
#include <QObject>
#include <QStringList>

#include <atomic>
#include <functional>

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

    const QString catalogueFileName = "catalogue.json";

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

}
}
