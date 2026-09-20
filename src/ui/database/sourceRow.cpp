// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "sourceRow.h"

#include "stepProgress.h"

namespace database
{

SourceRow::SourceRow(const QString &name, const QString &location,
    const QString &filter, QWidget *parent) :
    QWidget(parent), fileName(name), defaultLocation(location), fileFilter(
        filter)
{
  titleLabel = new QLabel(fileName, this);
  QFont font = titleLabel->font();
  font.setBold(true);
  titleLabel->setFont(font);

  locationEdit = new QLineEdit(defaultLocation, this);
  locationEdit->setPlaceholderText(tr("URL or local file path"));
  locationEdit->setClearButtonEnabled(true);

  browseButton = new QPushButton(tr("Browse..."), this);
  resetButton = new QPushButton(tr("Default"), this);
  stepProgress = new StepProgress(this);

  auto *grid = new QGridLayout(this);
  grid->setContentsMargins(0, 4, 0, 4);
  grid->addWidget(titleLabel, 0, 0, 1, 3);
  grid->addWidget(locationEdit, 1, 0);
  grid->addWidget(browseButton, 1, 1);
  grid->addWidget(resetButton, 1, 2);
  grid->addWidget(stepProgress, 2, 0, 1, 3);
  grid->setColumnStretch(0, 1);

  connect(browseButton, &QPushButton::clicked, this,
      &SourceRow::onBrowseClicked);
  connect(resetButton, &QPushButton::clicked, this, &SourceRow::onResetClicked);
}

QString SourceRow::getFileName() const
{
  return fileName;
}

QString SourceRow::getLocation() const
{
  return locationEdit->text().trimmed();
}

bool SourceRow::isLocal() const
{
  const QString text = getLocation();
  if (text.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)
      || text.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
    return false;
  }
  return true;
}

SourceSpec SourceRow::getSpec() const
{
  SourceSpec spec;
  spec.fileName = fileName;
  spec.location = getLocation();
  spec.isLocal = isLocal();
  return spec;
}

void SourceRow::setInputEnabled(bool enabled)
{
  locationEdit->setEnabled(enabled);
  browseButton->setEnabled(enabled);
  resetButton->setEnabled(enabled);
}

void SourceRow::setProgress(qint64 done, qint64 total)
{
  stepProgress->setProgress(done, total);
}

void SourceRow::setState(StepState state, const QString &message)
{
  stepProgress->setState(state, message);
}

void SourceRow::onBrowseClicked()
{
  QString startDir = QDir::homePath();
  if (isLocal() && !getLocation().isEmpty()) {
    startDir = QFileInfo(getLocation()).absolutePath();
  }

  const QString path = QFileDialog::getOpenFileName(this,
      tr("Select %1").arg(fileName), startDir, fileFilter);
  if (!path.isEmpty()) {
    locationEdit->setText(path);
  }
}

void SourceRow::onResetClicked()
{
  locationEdit->setText(defaultLocation);
}

}
