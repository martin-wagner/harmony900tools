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

struct SourceSpec
{
    QString fileName;
    QString location;   ///< url or local path
    bool isLocal = false;
};

/**
 * Everything the controller needs to start an import.
 * The order of sources matches the rows in the ui: the data files first,
 * the checksum file is stored separately.
 */
struct ImportConfig
{
    QString targetDir;
    QList<SourceSpec> sources;
};


/**
 * Runs the import: download (or use the local file), extract,
 * build the index. Files are processed one after another.
 *
 * Lives in the ui thread. Downloading is asynchronous (QNetworkAccessManager),
 * extracting and indexing run in worker threads (QtConcurrent).
 *
 * Source indices match ImportConfig::sources. Index -1 means the index step.
 */
class ImportDatabase : public QObject {
    Q_OBJECT

public:
    explicit ImportDatabase(QObject *parent = nullptr);
    ~ImportDatabase() override;

    bool isRunning() const;

signals:
    void sourceStateChanged(int index, StepState state, const QString &message);
    void sourceProgressChanged(int index, qint64 done, qint64 total);
    void indexStateChanged(StepState state, const QString &message);
    void indexProgressChanged(qint64 done, qint64 total);
    void logMessage(const QString &line);
    void finished(bool success);

public slots:
    void start(const ImportConfig &newConfig);
    void cancel();

private slots:
    void onDownloadReadyRead();
    void onDownloadProgress(qint64 received, qint64 total);
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
    std::function<bool(qint64, qint64)> createReporter(int index);

    /**
     * Placeholder for the catalogue index creation. Runs in a worker thread.
     * extractedFiles has the same order as ImportConfig::sources.
     */
    static bool createCatalogueIndex(const QStringList &extractedFiles, const QString &indexPath,
        const std::function<bool(qint64, qint64)> &progress);

    ImportConfig config;
    bool running = false;
    std::atomic<bool> cancelRequested{false};
    int currentIndex = -1;

    QStringList compressedPaths;
    QStringList extractedPaths;

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
