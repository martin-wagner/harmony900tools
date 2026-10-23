// SPDX-License-Identifier: LGPL-2.1-or-later

#include "deviceSearchWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

using namespace std;

namespace database
{

DeviceSearchWidget::DeviceSearchWidget(QWidget *parent) :
    QWidget(parent)
{
  createView();
  createConnections();
  updateDeviceInfo();
}

void DeviceSearchWidget::setCatalogue(
    const document::files::Database *catalogue)
{
  this->catalogue = catalogue;
  manufacturerEdit->clear();
  updateManufacturers();
  deviceEdit->clear();
  updateDevices();
  updateDeviceInfo();
}

void DeviceSearchWidget::onManufacturerTextChanged(const QString &text)
{
  updateManufacturers();
}

void DeviceSearchWidget::onManufacturerSelectionChanged()
{
  const QString manufacturerName = getSelectedManufacturer();
  deviceEdit->setEnabled(!manufacturerName.isEmpty());
  updateDevices();
  updateDeviceInfo();
}

void DeviceSearchWidget::onDeviceTextChanged(const QString &text)
{
  updateDevices();
}

void DeviceSearchWidget::onDeviceSelectionChanged()
{
  updateDeviceInfo();

  const QString manufacturerName = getSelectedManufacturer();
  const QString deviceName = getSelectedDevice();
  if (!manufacturerName.isEmpty() && !deviceName.isEmpty()) {
    emit deviceSelected(manufacturerName, deviceName);
  }
}

void DeviceSearchWidget::createView()
{
  manufacturerEdit = new QLineEdit(this);
  manufacturerEdit->setPlaceholderText(tr("search manufacturer"));

  manufacturerList = new QListWidget(this);
  manufacturerList->setSelectionMode(QAbstractItemView::SingleSelection);

  deviceEdit = new QLineEdit(this);
  deviceEdit->setPlaceholderText(tr("search device"));
  deviceEdit->setEnabled(false);

  deviceList = new QListWidget(this);
  deviceList->setSelectionMode(QAbstractItemView::SingleSelection);

  deviceInfoLabel = new QLabel(this);
  deviceInfoLabel->setWordWrap(true);

  QVBoxLayout *manufacturerLayout = new QVBoxLayout();
  manufacturerLayout->addWidget(manufacturerEdit);
  manufacturerLayout->addWidget(manufacturerList);

  QVBoxLayout *deviceLayout = new QVBoxLayout();
  deviceLayout->addWidget(deviceEdit);
  deviceLayout->addWidget(deviceList);

  QHBoxLayout *listsLayout = new QHBoxLayout();
  listsLayout->addLayout(manufacturerLayout);
  listsLayout->addLayout(deviceLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(listsLayout);
  mainLayout->addWidget(deviceInfoLabel);
}

void DeviceSearchWidget::createConnections()
{
  connect(manufacturerEdit, &QLineEdit::textChanged, this,
      &DeviceSearchWidget::onManufacturerTextChanged);
  connect(manufacturerList, &QListWidget::itemSelectionChanged, this,
      &DeviceSearchWidget::onManufacturerSelectionChanged);
  connect(deviceEdit, &QLineEdit::textChanged, this,
      &DeviceSearchWidget::onDeviceTextChanged);
  connect(deviceList, &QListWidget::itemSelectionChanged, this,
      &DeviceSearchWidget::onDeviceSelectionChanged);
}

void DeviceSearchWidget::updateManufacturers()
{
  manufacturerList->clear();
  if (catalogue == nullptr) {
    return;
  }

  auto matches = catalogue->findManufacturers(manufacturerEdit->text());
  matches.sort(Qt::CaseInsensitive);
  updateDeviceInfo(&matches);
  manufacturerList->addItems(matches);
}

void DeviceSearchWidget::updateDevices()
{
  deviceList->clear();
  const QString manufacturerName = getSelectedManufacturer();
  if (catalogue == nullptr || manufacturerName.isEmpty()) {
    return;
  }

  auto matches = catalogue->findDevices(manufacturerName, deviceEdit->text());
  matches.sort(Qt::CaseInsensitive);
  updateDeviceInfo(&matches);
  deviceList->addItems(matches);
}

void DeviceSearchWidget::updateDeviceInfo(QStringList *items)
{
  if (catalogue == nullptr) {
    deviceInfoLabel->setText(tr("No devices available!"));
    return;
  }
  if (items != nullptr) {
    deviceInfoLabel->setText(
        tr("%1 item(s) match your search").arg(items->size()));
    return;
  }

  const QString manufacturerName = getSelectedManufacturer();
  const QString deviceName = getSelectedDevice();
  if (manufacturerName.isEmpty() || deviceName.isEmpty()) {
    deviceInfoLabel->setText(
        tr("no device selected (%1 available)").arg(
            catalogue->getDeviceCount()));
    return;
  }

  const document::files::Database::Device info = catalogue->getDeviceInfo(
      manufacturerName, deviceName);
  if (info.name.isEmpty()) {
    deviceInfoLabel->setText(tr("device not found"));
    return;
  }

  deviceInfoLabel->setText(
      tr("%1 (Database type: %2)").arg(info.name).arg(info.type.getQString()));
}

QString DeviceSearchWidget::getSelectedManufacturer() const
{
  const QList<QListWidgetItem*> items = manufacturerList->selectedItems();
  if (items.isEmpty()) {
    return QString();
  }

  return items.first()->text();
}

QString DeviceSearchWidget::getSelectedDevice() const
{
  const QList<QListWidgetItem*> items = deviceList->selectedItems();
  if (items.isEmpty()) {
    return QString();
  }

  return items.first()->text();
}

}
