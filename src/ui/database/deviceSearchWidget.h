// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "document/files/database/importer.h"

class QLabel;
class QLineEdit;
class QListWidget;

namespace document
{
namespace files
{
class Database;
}
}

namespace database
{

/**
 * search widget for the device database: manufacturer filter, device filter
 * and device info. the device filter is enabled after a manufacturer is selected.
 */
class DeviceSearchWidget: public QWidget
{
  Q_OBJECT

  public:
    explicit DeviceSearchWidget(QWidget *parent = nullptr);

    /** sets the database to search in, nullptr clears the view */
    void setCatalogue(const document::files::Database *catalogue);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType) const;
    void writeMsg(const QString &message) const;
    void deviceSelected(const QString &manufacturerName, const QString &deviceName);

  private slots:
    void onManufacturerTextChanged(const QString &text);
    void onManufacturerSelectionChanged();
    void onDeviceTextChanged(const QString &text);
    void onDeviceSelectionChanged();

  private:
    void createView();
    void createConnections();

    void updateManufacturers();
    void updateDevices();
    void updateDeviceInfo(QStringList *items = nullptr);
    QString getSelectedManufacturer() const;
    QString getSelectedDevice() const;

    const document::files::Database *catalogue = nullptr;

    QLineEdit *manufacturerEdit = nullptr;
    QListWidget *manufacturerList = nullptr;
    QLineEdit *deviceEdit = nullptr;
    QListWidget *deviceList = nullptr;
    QLabel *deviceInfoLabel = nullptr;
};

}
