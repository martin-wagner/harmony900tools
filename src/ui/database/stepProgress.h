// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "document/files/database/loader.h"

class QLabel;
class QProgressBar;

namespace database
{

/**
 * Progress bar with a status text next to it.
 */
class StepProgress: public QWidget
{
  Q_OBJECT

  public:
    explicit StepProgress(QWidget *parent = nullptr);

    /** total <= 0 shows a busy indicator (unknown size). */
    void setProgress(qint64 done, qint64 total);
    void setState(document::files::StepState state, const QString &message = QString());

  private:
    QProgressBar *progressBar = nullptr;
    QLabel *statusLabel = nullptr;
};

}
