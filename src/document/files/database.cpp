// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QtConcurrent>

#include "database.h"

#include "lib/zip.h"

namespace document
{
namespace files
{

/** device_features_raw.jsonl.gz -> device_features_raw.jsonl */
QString getExtractedName(const QString &fileName)
{
  QString name = fileName;
  if (name.endsWith(QLatin1String(".gz"), Qt::CaseInsensitive)) {
    name.chop(3);
  } else {
    name += QStringLiteral(".extracted");
  }
  return name;
}

ImportDatabase::ImportDatabase(QObject *parent) :
    QObject(parent)
{
  network = new QNetworkAccessManager(this);

  connect(&extractWatcher, &QFutureWatcher<bool>::finished, this,
      &ImportDatabase::onExtractFinished);
  connect(&indexWatcher, &QFutureWatcher<bool>::finished, this,
      &ImportDatabase::onIndexFinished);
}

ImportDatabase::~ImportDatabase()
{
  cancelRequested = true;

  if (reply != nullptr) {
    reply->disconnect(this);
    reply->abort();
  }

  extractWatcher.disconnect(this);
  indexWatcher.disconnect(this);
  extractWatcher.waitForFinished();
  indexWatcher.waitForFinished();
}

bool ImportDatabase::isRunning() const
{
  return running;
}

void ImportDatabase::start(const ImportConfig &newConfig)
{
  if (running) {
    return;
  }

  config = newConfig;
  running = true;
  cancelRequested = false;
  currentIndex = -1;
  compressedPaths.clear();
  extractedPaths.clear();

  if (!QDir().mkpath(config.targetDir)) {
    emit logMessage(tr("Cannot create target folder %1").arg(config.targetDir));
    finish(false);
    return;
  }

  emit logMessage(tr("Importing into %1").arg(config.targetDir));
  processSource(0);
}

void ImportDatabase::cancel()
{
  if (!running) {
    return;
  }

  cancelRequested = true;

  // workers notice the flag in their progress callback,
  // a running download is aborted and ends in onDownloadFinished()
  if (reply != nullptr) {
    reply->abort();
  }
}

void ImportDatabase::onDownloadReadyRead()
{
  if (reply == nullptr || downloadWriteFailed) {
    return;
  }

  const QByteArray chunk = reply->readAll();
  if (downloadFile.write(chunk) != chunk.size()) {
    downloadWriteFailed = true;
    // not aborting directly, we may be called from onDownloadFinished()
    QMetaObject::invokeMethod(reply, "abort", Qt::QueuedConnection);
  }
}

void ImportDatabase::onDownloadProgress(qint64 received, qint64 total)
{
  if (reply != nullptr) {
    emit sourceProgressChanged(currentIndex, received, total);
  }
}

void ImportDatabase::onDownloadFinished()
{
  if (reply == nullptr) {
    return;
  }

  QNetworkReply *finishedReply = reply;
  if (finishedReply->error() == QNetworkReply::NoError) {
    onDownloadReadyRead(); // flush remaining data
  }
  reply = nullptr;

  const QNetworkReply::NetworkError error = finishedReply->error();
  const QString errorText = finishedReply->errorString();
  finishedReply->deleteLater();
  downloadFile.close();

  const int index = currentIndex;
  const bool failed = cancelRequested.load() || downloadWriteFailed
      || error != QNetworkReply::NoError;
  if (failed) {
    downloadFile.remove();
    if (cancelRequested.load()) {
      fail(index, tr("Cancelled"));
    } else if (downloadWriteFailed) {
      fail(index, tr("Cannot write to disk"));
    } else {
      fail(index, errorText);
    }
    return;
  }

  QFile::remove(downloadTargetPath);
  if (!downloadFile.rename(downloadTargetPath)) {
    fail(index, downloadFile.errorString());
    return;
  }
  emit logMessage(tr("Downloaded %1").arg(downloadTargetPath));

  compressedPaths.append(downloadTargetPath);
  extractFile(index);
}

void ImportDatabase::onExtractFinished()
{
  const int index = currentIndex;

  if (cancelRequested.load()) {
    fail(index, tr("Cancelled"));
    return;
  }

  if (!extractWatcher.result()) {
    fail(index, tr("Extraction failed"));
    return;
  }
  finishSource(index);
}

void ImportDatabase::onIndexFinished()
{
  if (cancelRequested.load()) {
    fail(-1, tr("Cancelled"));
    return;
  }

  if (!indexWatcher.result()) {
    fail(-1, tr("Index creation failed"));
    return;
  }

  emit indexStateChanged(StepState::Done, tr("Done"));
  emit logMessage(tr("Import finished"));
  finish(true);
}

void ImportDatabase::processSource(int index)
{
  if (index >= config.sources.size()) {
    buildIndex();
    return;
  }

  currentIndex = index;
  const SourceSpec &spec = config.sources.at(index);

  if (spec.isLocal) {
    emit logMessage(tr("Using local file %1").arg(spec.location));
    compressedPaths.append(spec.location);
    extractFile(index);
    return;
  }

  startDownload(index, spec.location,
      config.targetDir + QLatin1Char('/') + spec.fileName);
}

void ImportDatabase::startDownload(int index, const QString &location,
    const QString &targetPath)
{
  currentIndex = index;
  downloadTargetPath = targetPath;
  downloadWriteFailed = false;

  downloadFile.setFileName(targetPath + QStringLiteral(".part"));
  if (!downloadFile.open(QIODevice::WriteOnly)) {
    fail(index, tr("Cannot write %1").arg(downloadFile.fileName()));
    return;
  }

  emit sourceStateChanged(index, StepState::Running, tr("Downloading"));
  emit logMessage(tr("Downloading %1").arg(location));

  const QUrl url(location);
  QNetworkRequest request(url);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
      QNetworkRequest::NoLessSafeRedirectPolicy);

  reply = network->get(request);
  connect(reply, &QNetworkReply::readyRead, this,
      &ImportDatabase::onDownloadReadyRead);
  connect(reply, &QNetworkReply::downloadProgress, this,
      &ImportDatabase::onDownloadProgress);
  connect(reply, &QNetworkReply::finished, this,
      &ImportDatabase::onDownloadFinished);
}

void ImportDatabase::extractFile(int index)
{
  currentIndex = index;
  const QString source = compressedPaths.at(index);
  const QString target = getExtractedPath(index);

  // a local file that already is the extracted target, nothing to do
  if (QFileInfo(source).absoluteFilePath()
      == QFileInfo(target).absoluteFilePath()) {
    emit logMessage(tr("%1 is already extracted").arg(source));
    finishSource(index);
    return;
  }

  emit sourceStateChanged(index, StepState::Running, tr("Extracting"));
  emit logMessage(tr("Extracting %1").arg(source));

  const std::function<bool(qint64, qint64)> reporter = createReporter(index);
  extractWatcher.setFuture(QtConcurrent::run([source, target, reporter]() {
    return lib::gunzipToFile(source, target, reporter);
  }));
}

void ImportDatabase::finishSource(int index)
{
  extractedPaths.append(getExtractedPath(index));
  emit sourceStateChanged(index, StepState::Done, tr("Done"));
  processSource(index + 1);
}

void ImportDatabase::buildIndex()
{
  currentIndex = -1;
  emit indexStateChanged(StepState::Running, tr("Building index"));
  emit logMessage(tr("Building catalogue index"));

  const QStringList files = extractedPaths;
  const QString indexPath = config.targetDir
      + QStringLiteral("/catalogue.index");
  const std::function<bool(qint64, qint64)> reporter = createReporter(-1);
  indexWatcher.setFuture(QtConcurrent::run([files, indexPath, reporter]() {
    return ImportDatabase::createCatalogueIndex(files, indexPath, reporter);
  }));
}

void ImportDatabase::fail(int index, const QString &message)
{
  if (index >= 0) {
    emit sourceStateChanged(index, StepState::Failed, message);
  } else {
    emit indexStateChanged(StepState::Failed, message);
  }

  if (cancelRequested.load()) {
    emit logMessage(tr("Cancelled"));
  } else {
    emit logMessage(tr("Error: %1").arg(message));
  }
  finish(false);
}

void ImportDatabase::finish(bool success)
{
  if (downloadFile.isOpen()) {
    downloadFile.close();
  }
  running = false;
  emit finished(success);
}

QString ImportDatabase::getExtractedPath(int index) const
{
  return config.targetDir + QLatin1Char('/')
      + getExtractedName(config.sources.at(index).fileName);
}

std::function<bool(qint64, qint64)> ImportDatabase::createReporter(int index)
{
  QElapsedTimer timer;
  return [this, index, timer](qint64 done, qint64 total) mutable
  {
    if (!timer.isValid() || timer.elapsed() >= 100) {
      timer.start();
      QMetaObject::invokeMethod(this, [this, index, done, total]() {
        if (index >= 0) {
          emit sourceProgressChanged(index, done, total);
        } else {
          emit indexProgressChanged(done, total);
        }
      }, Qt::QueuedConnection);
    }
    return !cancelRequested.load();
  };
}

bool ImportDatabase::createCatalogueIndex(const QStringList &extractedFiles,
    const QString &indexPath,
    const std::function<bool(qint64, qint64)> &progress)
{
  Q_UNUSED(extractedFiles)
  Q_UNUSED(indexPath)
  Q_UNUSED(progress)

  // TODO: read the catalogue file line by line and store manufacturer,
  // device name, device id and line number in the index at indexPath.
  // Call progress(bytesRead, fileSize) regularly and stop (return false)
  // if it returns false.
  return true;
}

}
}
