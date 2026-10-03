// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "document/files/databaseLoader.h"


class QLabel;
class QLineEdit;
class QPushButton;

namespace database
{

class StepProgress;

/**
 * One file to import: name, editable location (url or local path),
 * browse / reset buttons and a progress display.
 */
class SourceRow: public QWidget
{
  Q_OBJECT

  public:
    SourceRow(const QString &name, const QString &location,
        const QString &filter, QWidget *parent = nullptr);

    QString getFileName() const;
    QString getLocation() const;
    bool isLocal() const;
    document::files::SourceSpec getSpec() const;

    void setInputEnabled(bool enabled);
    void setProgress(qint64 done, qint64 total);
    void setState(document::files::StepState state, const QString &message = QString());

  private slots:
    void onBrowseClicked();
    void onResetClicked();

  private:
    QString fileName;
    QString defaultLocation;
    QString fileFilter;

    QLabel *titleLabel = nullptr;
    QLineEdit *locationEdit = nullptr;
    QPushButton *browseButton = nullptr;
    QPushButton *resetButton = nullptr;
    StepProgress *stepProgress = nullptr;
};

}
