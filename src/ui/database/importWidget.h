// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QList>
#include <QWidget>

#include "ui/logViewer.h"
#include "document/files/database.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

namespace database
{

class SourceRow;
class StepProgress;

/**
 * UI for importing the database: target folder, sources (url or local file),
 * optional checksum verification, index progress and a log.
 *
 * Source indices: 0..2 are the data files, 3 is the checksum file.
 */
class ImportWidget: public QWidget
{
  Q_OBJECT

  public:
    explicit ImportWidget(QWidget *parent = nullptr);

    const QString releaseBaseUrl =
        "https://github.com/pickysysadmin/logitech-harmony-ir-archive/releases/download/raw-2026-09-02/";
    const QList<QPair<document::files::DatabaseCatalogueFileType, QString>> dataFiles = {
      {document::files::DatabaseCatalogueFileType::Raw, "catalog_raw.jsonl.gz"},
      {document::files::DatabaseCatalogueFileType::Devices, "device_features_raw.jsonl.gz"},
      {document::files::DatabaseCatalogueFileType::Protocols, "AllProtocols.json.gz"},
    };
    static constexpr uint64_t requiredBytes = 7ull * 1024 * 1024 * 1024;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType) const;
    void writeMsg(const QString &message) const;

    void importDone(const document::files::DatabasePaths &paths);
    void cancelRequested();

  public slots:
    void setSourceState(int index, document::files::StepState state, const QString &message = QString());
    void setSourceProgress(int index, qint64 done, qint64 total);
    void setIndexState(document::files::StepState state, const QString &message = "");
    void setIndexProgress(qint64 done, qint64 total);
    void appendLog(const QString &line);
    void done(bool success, const document::files::DatabasePaths paths);

    /** The controller calls setBusy(false) when everything is finished or cancelled. */
    void setBusy(bool busy);

  private slots:
    void onTargetBrowseClicked();
    void onImportClicked();
    void onCancelClicked();

  private:
    void createView();
    void createConnections();

    void updateSpaceInfo();
    qint64 getAvailableBytes() const;
    bool validateInput(QString &errorText) const;
    document::files::ImporterConfig buildConfig() const;
    void resetStates();

    QLineEdit *targetEdit = nullptr;
    QPushButton *targetBrowseButton = nullptr;
    QLabel *spaceLabel = nullptr;

    QList<SourceRow*> sourceRows;

    StepProgress *indexProgress = nullptr;

    QPlainTextEdit *logView = nullptr;
    QPushButton *importButton = nullptr;
    QPushButton *cancelButton = nullptr;

  private:
    document::files::ImportDatabase *databaseImporter = nullptr;
};

}
