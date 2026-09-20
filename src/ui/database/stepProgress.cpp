// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

#include "stepProgress.h"

namespace database
{

namespace
{

constexpr int progressSteps = 1000;

QString stateToText(StepState state)
{
  switch (state) {
    case StepState::Idle:
      return QCoreApplication::translate("ui::StepProgress", "Ready");
    case StepState::Running:
      return QCoreApplication::translate("ui::StepProgress", "Working...");
    case StepState::Done:
      return QCoreApplication::translate("ui::StepProgress", "Done");
    case StepState::Failed:
      return QCoreApplication::translate("ui::StepProgress", "Failed");
    case StepState::Skipped:
      return QCoreApplication::translate("ui::StepProgress", "Skipped");
  }
  return QString();
}

}

StepProgress::StepProgress(QWidget *parent) :
    QWidget(parent)
{
  progressBar = new QProgressBar(this);
  progressBar->setRange(0, progressSteps);
  progressBar->setValue(0);

  statusLabel = new QLabel(stateToText(StepState::Idle), this);
  statusLabel->setMinimumWidth(160);

  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(progressBar, 1);
  layout->addWidget(statusLabel);
}

void StepProgress::setProgress(qint64 done, qint64 total)
{
  if (total <= 0) {
    progressBar->setRange(0, 0);
    return;
  }
  progressBar->setRange(0, progressSteps);
  progressBar->setValue(static_cast<int>((done * progressSteps) / total));
}

void StepProgress::setState(StepState state, const QString &message)
{
  QString text = stateToText(state);
  if (!message.isEmpty()) {
    text = message;
  }
  statusLabel->setText(text);

  if (state == StepState::Done) {
    progressBar->setRange(0, progressSteps);
    progressBar->setValue(progressSteps);
  } else if (state == StepState::Idle) {
    progressBar->setRange(0, progressSteps);
    progressBar->setValue(0);
  } else if (state != StepState::Running && progressBar->maximum() == 0) {
    // leave the busy indicator state
    progressBar->setRange(0, progressSteps);
    progressBar->setValue(0);
  }
}

}
