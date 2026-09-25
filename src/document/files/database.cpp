// SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>
#include <map>
#include <vector>

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QtConcurrent>
#include <QSaveFile>

#include <nlohmann/json.hpp>

#include "database.h"

#include "lib/zip.h"

namespace document
{
namespace files
{

/**
 * Sort order for manufacturers and devices: case insensitive first,
 * ties are broken case sensitive. A search has to use the same order.
 */
struct NameLess
{
    bool operator()(const QString &a, const QString &b) const
    {
      const int folded = a.compare(b, Qt::CaseInsensitive);
      if (folded != 0) {
        return folded < 0;
      }
      return a.compare(b, Qt::CaseSensitive) < 0;
    }
};

struct DeviceEntry
{
    QString name;
    uint64_t id = 0;
    uint64_t line = 0;
};

using Catalogue = std::map<QString, std::vector<DeviceEntry>, NameLess>;

/**
 * SAX handler that only picks the three wanted top level keys of one line.
 * Nothing is stored for the rest of the document, so no DOM is built.
 * Nested keys (e.g. "Id" inside arrays) are ignored via the depth check.
 */
class CatalogueLineHandler: public nlohmann::json_sax<nlohmann::json>
{
  public:
    using Json = nlohmann::json;

    enum class Field
    {
      None, Manufacturer, Model, Id
    };

    QString manufacturer;
    QString model;
    uint64_t id = 0;
    bool hasId = false;

    void reset()
    {
      manufacturer.clear();
      model.clear();
      id = 0;
      hasId = false;
      depth = 0;
      currentField = Field::None;
    }

    bool isValid() const
    {
      return (!manufacturer.isEmpty() && !model.isEmpty() && hasId);
    }

    bool null() override
    {
      return true;
    }

    bool boolean(bool) override
    {
      return true;
    }

    bool number_integer(Json::number_integer_t value) override
    {
      storeNumber(static_cast<uint64_t>(value));
      return true;
    }

    bool number_unsigned(Json::number_unsigned_t value) override
    {
      storeNumber(static_cast<uint64_t>(value));
      return true;
    }

    bool number_float(Json::number_float_t, const Json::string_t&) override
    {
      return true;
    }

    bool string(Json::string_t &value) override
    {
      if (depth == 1) {
        if (currentField == Field::Manufacturer) {
          manufacturer = QString::fromUtf8(value.data(),
              static_cast<int>(value.size()));
        } else if (currentField == Field::Model) {
          model = QString::fromUtf8(value.data(),
              static_cast<int>(value.size()));
        }
      }
      return true;
    }

    bool binary(Json::binary_t&) override
    {
      return true;
    }

    bool start_object(std::size_t) override
    {
      depth++;
      return true;
    }

    bool key(Json::string_t &name) override
    {
      currentField = Field::None;
      if (depth == 1) {
        if (name == "PrimaryManufacturerAlias") {
          currentField = Field::Manufacturer;
        } else if (name == "PrimaryModelAlias") {
          currentField = Field::Model;
        } else if (name == "Id") {
          currentField = Field::Id;
        }
      }
      return true;
    }

    bool end_object() override
    {
      depth--;
      return true;
    }

    bool start_array(std::size_t) override
    {
      depth++;
      return true;
    }

    bool end_array() override
    {
      depth--;
      return true;
    }

    bool parse_error(std::size_t, const std::string&, const Json::exception&)
    override
    {
      return false;
    }

  private:
    void storeNumber(uint64_t value)
    {
      if ((depth == 1) && (currentField == Field::Id)) {
        id = value;
        hasId = true;
      }
    }

    int depth = 0;
    Field currentField = Field::None;
};

/** Returns value as quoted / escaped json string. */
QByteArray jsonString(const QString &value)
{
  const QByteArray utf8 = value.toUtf8();
  const nlohmann::json json = std::string(utf8.constData(),
      static_cast<size_t>(utf8.size()));
  const std::string text = json.dump(-1, ' ', false,
      nlohmann::json::error_handler_t::replace);
  return QByteArray(text.data(), static_cast<int>(text.size()));
}

/** Writes the index (streamed, no json document in memory). Atomic via QSaveFile. */
bool writeIndex(const QString &indexPath, const QString &sourceName,
    uint64_t lineCount, const Catalogue &catalogue, QString &errorText)
{
  static const int FLUSH_SIZE = 1 << 20;

  QSaveFile out(indexPath);
  if (!out.open(QIODevice::WriteOnly)) {
    errorText = QStringLiteral("Cannot write %1").arg(indexPath);
    return false;
  }

  QByteArray buffer;
  buffer.reserve(FLUSH_SIZE * 2);
  bool ok = true;

  const auto flush = [&]() {
    if (ok && out.write(buffer) != buffer.size()) {
      ok = false;
    }
    buffer.clear();
  };

  buffer += "{\n  \"Version\": 1,\n  \"Source\": " + jsonString(sourceName)
      + ",\n  \"LineCount\": " + QByteArray::number(lineCount)
      + ",\n  \"Manufacturers\": [\n";

  bool firstManufacturer = true;
  for (const auto &entry : catalogue) {
    if (!firstManufacturer) {
      buffer += ",\n";
    }
    firstManufacturer = false;

    buffer += "    {\n      \"Name\": " + jsonString(entry.first)
        + ",\n      \"Devices\": [\n";

    bool firstDevice = true;
    for (const DeviceEntry &device : entry.second) {
      if (!firstDevice) {
        buffer += ",\n";
      }
      firstDevice = false;

      buffer += "        { \"Name\": " + jsonString(device.name) + ", \"Id\": "
          + QByteArray::number(device.id) + ", \"Line\": "
          + QByteArray::number(device.line) + " }";

      if (buffer.size() > FLUSH_SIZE) {
        flush();
      }
    }
    buffer += "\n      ]\n    }";
  }

  buffer += "\n  ]\n}\n";
  flush();

  if (!ok) {
    errorText = QStringLiteral("Write error on %1").arg(indexPath);
    out.cancelWriting();
    return false;
  }
  if (!out.commit()) {
    errorText = QStringLiteral("Cannot finalize %1").arg(indexPath);
    return false;
  }
  return true;
}

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

void ImportDatabase::start(const ImporterConfig &newConfig)
{
  if (running) {
    return;
  }

  config = newConfig;
  running = true;
  cancelRequested = false;
  currentIndex = -1;
  compressedPaths.clear();
  extractedPaths = DatabasePaths();

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

void ImportDatabase::onDownloadProgress(uint64_t received, uint64_t total)
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
    // done
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

  //don't check if file exists -- might be partial/cancelled or failed.
  //we most likely extract once per install lifetime, so doesn't matter.

  emit sourceStateChanged(index, StepState::Running, tr("Extracting"));
  emit logMessage(tr("Extracting %1").arg(source));

  const std::function<bool(uint64_t, uint64_t)> reporter = createReporter(
      index);
  extractWatcher.setFuture(QtConcurrent::run([source, target, reporter]() {
    return lib::gunzipToFile(source, target, reporter);
  }));
}

void ImportDatabase::finishSource(int index)
{
  auto path = getExtractedPath(index);

  switch (config.sources[index].type) {
    case DatabaseCatalogueFileType::Raw:
      extractedPaths.raw = path;
      break;
    case DatabaseCatalogueFileType::Devices:
      extractedPaths.devices = path;
      break;
    case DatabaseCatalogueFileType::Protocols:
      extractedPaths.protocols = path;
      break;
    default:
      break;
  }
  emit sourceStateChanged(index, StepState::Done, tr("Done"));
  processSource(index + 1);
}

void ImportDatabase::buildIndex()
{
  currentIndex = -1;
  emit indexStateChanged(StepState::Running, tr("Building index"));
  emit logMessage(tr("Building catalogue index"));

  extractedPaths.catalogue = config.targetDir + "/" + catalogueFileName;
  const auto files = extractedPaths;
  const std::function<bool(uint64_t, uint64_t)> reporter = createReporter(-1);
  indexWatcher.setFuture(QtConcurrent::run([this, files, reporter]() {
    return createCatalogueIndex(files, reporter);
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
  DatabasePaths files;

  if (downloadFile.isOpen()) {
    downloadFile.close();
  }
  running = false;

  if (success) {
    files = extractedPaths;
  }
  emit finished(success, files);
}

QString ImportDatabase::getExtractedPath(int index) const
{
  return config.targetDir + QLatin1Char('/')
      + getExtractedName(config.sources.at(index).fileName);
}

std::function<bool(uint64_t, uint64_t)> ImportDatabase::createReporter(
    int index)
{
  QElapsedTimer timer;
  return [this, index, timer](uint64_t done, uint64_t total) mutable
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

bool ImportDatabase::createCatalogueIndex(const DatabasePaths &paths,
    const std::function<bool(uint64_t, uint64_t)> &progress)
{
  static const uint64_t PROGRESS_INTERVAL = 512; // lines
  uint64_t bytesRead = 0;
  uint64_t lineCount = 0;
  uint64_t invalidLines = 0;
  uint64_t incompleteLines = 0;
  Catalogue catalogue;
  CatalogueLineHandler handler;

  // runs in a worker thread, so the log message is posted to the object's thread
  const auto log = [this](const QString &text) {
    QMetaObject::invokeMethod(this, [this, text]() {
          emit logMessage(text);
        }, Qt::QueuedConnection);
  };

  // find the catalogue file
  if (paths.raw.isEmpty()) {
    log(tr("Catalogue file not found"));
    return false;
  }

  QFile file(paths.raw);
  if (!file.open(QIODevice::ReadOnly)) {
    log(tr("Cannot open %1").arg(paths.raw));
    return false;
  }

  // read and collect
  const auto fileSize = file.size();

  while (!file.atEnd()) {
    const QByteArray line = file.readLine();
    bytesRead += line.size();

    const char *begin = line.constData();
    const char *end = begin + line.size();
    while ((end > begin) && ((end[-1] == '\n') || (end[-1] == '\r'))) {
      end--;
    }

    if (end != begin) {
      handler.reset();
      const bool parsed = nlohmann::json::sax_parse(begin, end, &handler);

      if (!parsed) {
        invalidLines++;
      } else if (!handler.isValid()) {
        incompleteLines++;
      } else {
        DeviceEntry device;
        device.name = handler.model;
        device.id = handler.id;
        device.line = lineCount;
        catalogue[handler.manufacturer].push_back(device);
      }
    }
    lineCount++;

    if ((progress != nullptr) && ((lineCount % PROGRESS_INTERVAL) == 0)
        && !progress(bytesRead, fileSize)) {
      return false;
    }
  }
  file.close();

  if ((invalidLines > 0) || (incompleteLines > 0)) {
    log(tr("Skipped %1 invalid lines and %2 "
        "lines without manufacturer, "
        "model or id").arg(invalidLines).arg(incompleteLines));
  }

  // sort devices, keep the first of duplicate manufacturer / device pairs
  uint64_t duplicates = 0;
  for (auto &entry : catalogue) {
    std::vector<DeviceEntry> &devices = entry.second;

    std::stable_sort(devices.begin(), devices.end(),
        [](const DeviceEntry &a, const DeviceEntry &b) {
          return NameLess()(a.name, b.name);
        });

    const auto last = std::unique(devices.begin(), devices.end(),
        [](const DeviceEntry &a, const DeviceEntry &b) {
          return a.name == b.name;
        });
    duplicates += std::distance(last, devices.end());
    devices.erase(last, devices.end());
  }
  if (duplicates > 0) {
    log(tr("Dropped %1 duplicate manufacturer / "
        "device entries").arg(duplicates));
  }

  if (progress != nullptr && !progress(fileSize, fileSize)) {
    return false;
  }

  QString errorText;
  if (!writeIndex(paths.catalogue, QFileInfo(paths.raw).fileName(), lineCount,
      catalogue, errorText)) {
    log(errorText);
    return false;
  }
  return true;
}

}
}
