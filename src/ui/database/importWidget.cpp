// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStorageInfo>
#include <QVBoxLayout>

#include "importWidget.h"

#include "sourceRow.h"
#include "stepProgress.h"

using namespace std;
using namespace document::files;

namespace database
{

ImportWidget::ImportWidget(QWidget *parent) :
    QWidget(parent)
{
  createView();
  createConnections();
  updateSpaceInfo();
}

void ImportWidget::setSourceState(int index, StepState state,
    const QString &message)
{
  if ((index >= 0) && (index < sourceRows.size())) {
    sourceRows.at(index)->setState(state, message);
  }
}

void ImportWidget::setSourceProgress(int index, qint64 done, qint64 total)
{
  if ((index >= 0) && (index < sourceRows.size())) {
    sourceRows.at(index)->setProgress(done, total);
  }
}

void ImportWidget::setIndexState(StepState state, const QString &message)
{
  indexProgress->setState(state, message);
}

void ImportWidget::setIndexProgress(qint64 done, qint64 total)
{
  indexProgress->setProgress(done, total);
}

void ImportWidget::appendLog(const QString &line)
{
  logView->appendPlainText(line);
  emit writeLog(LogLevel::Info, line, ContentType::PlainText);
}

void ImportWidget::setBusy(bool busy)
{
  const bool idle = !busy;

  targetEdit->setEnabled(idle);
  targetBrowseButton->setEnabled(idle);

  for (int i = 0; i < sourceRows.size(); i++) {
    sourceRows.at(i)->setInputEnabled(idle);
  }

  importButton->setEnabled(idle);
  cancelButton->setEnabled(busy);

  if (idle) {
    databaseImporter->deleteLater();
    databaseImporter = nullptr;

    importDone(buildConfig());
  }
}

void ImportWidget::onTargetBrowseClicked()
{
  QString startDir = targetEdit->text().trimmed();
  if (startDir.isEmpty()) {
    startDir = QDir::homePath();
  }

  const QString dir = QFileDialog::getExistingDirectory(this,
      tr("Select target folder"), startDir);
  if (!dir.isEmpty()) {
    targetEdit->setText(dir);
  }
}

void ImportWidget::onImportClicked()
{
  QString errorText;
  if (!validateInput(errorText)) {
    QMessageBox::warning(this, tr("Import"), errorText);
    return;
  }

  const qint64 available = getAvailableBytes();
  if ((available >= 0) && (available < requiredBytes)) {
    const QMessageBox::StandardButton answer = QMessageBox::question(this,
        tr("Import"),
        tr("There is probably not enough free space in the target "
            "folder. Continue anyway?"));
    if (answer != QMessageBox::Yes) {
      return;
    }
  }

  resetStates();
  setBusy(true);

  if (databaseImporter != nullptr) {
    databaseImporter->deleteLater();
  }
  databaseImporter = new ImportDatabase(this);

  connect(this, &ImportWidget::cancelRequested, databaseImporter,
      &ImportDatabase::cancel);

  connect(databaseImporter, &ImportDatabase::sourceStateChanged, this,
      &ImportWidget::setSourceState);
  connect(databaseImporter, &ImportDatabase::sourceProgressChanged, this,
      &ImportWidget::setSourceProgress);
  connect(databaseImporter, &ImportDatabase::indexStateChanged, this,
      &ImportWidget::setIndexState);
  connect(databaseImporter, &ImportDatabase::indexProgressChanged, this,
      &ImportWidget::setIndexProgress);
  connect(databaseImporter, &ImportDatabase::logMessage, this,
      &ImportWidget::appendLog);
  connect(databaseImporter, &ImportDatabase::finished, this,
      [this](bool) {setBusy(false);});

  databaseImporter->start(buildConfig());
}

void ImportWidget::onCancelClicked()
{
  if (databaseImporter == nullptr) {
    return;
  }

  cancelButton->setEnabled(false);
  appendLog(tr("Cancelling..."));
  emit cancelRequested();
}

void ImportWidget::createView()
{
  const QString gzipFilter = tr("Gzip files (*.gz);;All files (*)");
  const QString anyFilter = tr("All files (*)");

  // storage
  targetEdit = new QLineEdit(this);
  targetEdit->setPlaceholderText(
      tr("Folder for downloaded and extracted files"));
  targetBrowseButton = new QPushButton(tr("Browse..."), this);
  spaceLabel = new QLabel(this);

  auto *storageLayout = new QGridLayout;
  storageLayout->addWidget(targetEdit, 0, 0);
  storageLayout->addWidget(targetBrowseButton, 0, 1);
  storageLayout->addWidget(spaceLabel, 1, 0, 1, 2);
  storageLayout->setColumnStretch(0, 1);

  auto *storageGroup = new QGroupBox(tr("Storage"), this);
  storageGroup->setLayout(storageLayout);

  // sources
  for (const QString &name : dataFiles) {
    sourceRows.append(
        new SourceRow(name, releaseBaseUrl + name, gzipFilter, this));
  }
  auto *sourcesLayout = new QVBoxLayout;
  for (int i = 0; i < sourceRows.size(); i++) {
    sourcesLayout->addWidget(sourceRows.at(i));
  }

  auto *sourcesGroup = new QGroupBox(tr("Sources"), this);
  sourcesGroup->setLayout(sourcesLayout);

  // index
  indexProgress = new StepProgress(this);

  auto *indexLayout = new QVBoxLayout;
  indexLayout->addWidget(indexProgress);

  auto *indexGroup = new QGroupBox(tr("Index"), this);
  indexGroup->setLayout(indexLayout);

  // log
  logView = new QPlainTextEdit(this);
  logView->setReadOnly(true);
  logView->setMaximumBlockCount(10000);
  logView->setMinimumHeight(120);

  // buttons
  importButton = new QPushButton(tr("Download and build index"), this);
  cancelButton = new QPushButton(tr("Cancel"), this);
  cancelButton->setEnabled(false);

  auto *buttonLayout = new QHBoxLayout;
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(importButton);
  buttonLayout->addWidget(cancelButton);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(storageGroup);
  mainLayout->addWidget(sourcesGroup);
  mainLayout->addWidget(indexGroup);
  mainLayout->addWidget(logView, 1);
  mainLayout->addLayout(buttonLayout);
}

void ImportWidget::createConnections()
{
  connect(targetEdit, &QLineEdit::textChanged, this,
      &ImportWidget::updateSpaceInfo);
  connect(targetBrowseButton, &QPushButton::clicked, this,
      &ImportWidget::onTargetBrowseClicked);
  connect(importButton, &QPushButton::clicked, this,
      &ImportWidget::onImportClicked);
  connect(cancelButton, &QPushButton::clicked, this,
      &ImportWidget::onCancelClicked);
}

void ImportWidget::updateSpaceInfo()
{
  const QString required = QLocale().formattedDataSize(requiredBytes);
  const qint64 available = getAvailableBytes();

  if (available < 0) {
    spaceLabel->setText(tr("Required space: approx. %1").arg(required));
    return;
  }

  QString text = tr("Required space: approx. %1, available: %2").arg(required,
      QLocale().formattedDataSize(available));
  if (available < requiredBytes) {
    text += tr(" - not enough space");
  }
  spaceLabel->setText(text);
}

qint64 ImportWidget::getAvailableBytes() const
{
  const QString path = targetEdit->text().trimmed();
  if (path.isEmpty()) {
    return -1;
  }

  // the target folder may not exist yet, use the closest existing parent
  QDir dir(path);
  while (!dir.exists()) {
    if (!dir.cdUp()) {
      break;
    }
  }

  const QStorageInfo info(dir.absolutePath());
  if (!info.isValid() || !info.isReady()) {
    return -1;
  }
  return info.bytesAvailable();
}

bool ImportWidget::validateInput(QString &errorText) const
{
  if (targetEdit->text().trimmed().isEmpty()) {
    errorText = tr("Please select a target folder.");
    return false;
  }

  for (int i = 0; i < sourceRows.size(); i++) {
    const SourceRow *row = sourceRows.at(i);
    if (row->getLocation().isEmpty()) {
      errorText = tr("Missing location for %1.").arg(row->getFileName());
      return false;
    }
    if (row->isLocal() && !QFileInfo::exists(row->getLocation())) {
      errorText = tr("File not found: %1").arg(row->getLocation());
      return false;
    }
  }
  return true;
}

ImportConfig ImportWidget::buildConfig() const
{
  ImportConfig config;
  config.targetDir = targetEdit->text().trimmed();
  for (int i = 0; i < sourceRows.size(); i++) {
    config.sources.append(sourceRows.at(i)->getSpec());
  }
  return config;
}

void ImportWidget::resetStates()
{
  for (int i = 0; i < sourceRows.size(); i++) {
    sourceRows.at(i)->setState(StepState::Idle);
  }
  indexProgress->setState(StepState::Idle);
  logView->clear();
}

}
