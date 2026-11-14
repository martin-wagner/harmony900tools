// SPDX-License-Identifier: LGPL-2.1-or-later

#include "deviceSearchWidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

using namespace std;

namespace database
{

DeviceSearchWidget::DeviceSearchWidget(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx)
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
  updateImportButton();
}

void DeviceSearchWidget::onManufacturerTextChanged(const QString &text)
{
  updateManufacturers();
}

void DeviceSearchWidget::onManufacturerSelectionChanged()
{
  updateDevices();
  updateDeviceInfo();
  updateImportButton();
}

void DeviceSearchWidget::onDeviceTextChanged(const QString &text)
{
  updateDevices();
}

void DeviceSearchWidget::onDeviceSelectionChanged()
{
  QString manufacturerName = getSelectedManufacturer();
  const QString deviceName = getSelectedDevice();

  if (manufacturerName.isEmpty() && !deviceName.isEmpty()) {
    selectManufacturerOfDevice(deviceName);
    manufacturerName = getSelectedManufacturer();
  }

  updateDeviceInfo();
  updateImportButton();

  if (!manufacturerName.isEmpty() && !deviceName.isEmpty()) {
    emit deviceSelected(manufacturerName, deviceName);
  }
}

void DeviceSearchWidget::onImportClicked()
{
  const auto manufacturerName = getSelectedManufacturer();
  const auto deviceName = getSelectedDevice();
  const auto tryConvertProtocol = tryConvertProtocolCheck->isChecked();
  auto config = ctx.config();
  auto ok = false;

  if ((catalogue == nullptr) || (config == nullptr)) {
    emit writeMsg(tr("import not possible: no database or no document open"));
    return;
  }
  if (manufacturerName.isEmpty() || deviceName.isEmpty()) {
    return;
  }

  // needs the catalogue check from above, so it can't be declared earlier
  const auto source = catalogue->getDeviceInfo(manufacturerName, deviceName);
  if (source.name.isEmpty()) {
    emit writeMsg(tr("device not found"));
    return;
  }

  ok = catalogue->importDevice(source, *config, tryConvertProtocol);
  if (ok) {
    emit writeMsg(tr("imported %1 / %2").arg(manufacturerName, deviceName));
  } else {
    emit writeMsg(
        tr("import of %1 / %2 is incomplete or failed").arg(manufacturerName,
            deviceName));
  }
}

void DeviceSearchWidget::onClearManufacturerClicked()
{
  manufacturerList->clearSelection();
  manufacturerEdit->clear();
}

void DeviceSearchWidget::createView()
{
  manufacturerEdit = new QLineEdit(this);
  manufacturerEdit->setPlaceholderText(tr("search manufacturer"));

  clearManufacturerButton = new QPushButton(tr("all"), this);
  clearManufacturerButton->setToolTip(tr("Deselect the manufacturer"));

  manufacturerList = new QListWidget(this);
  manufacturerList->setSelectionMode(QAbstractItemView::SingleSelection);

  deviceEdit = new QLineEdit(this);
  deviceEdit->setPlaceholderText(tr("search device"));

  deviceList = new QListWidget(this);
  deviceList->setSelectionMode(QAbstractItemView::SingleSelection);

  deviceInfoLabel = new QLabel(this);
  deviceInfoLabel->setWordWrap(true);

  tryConvertProtocolCheck = new QCheckBox(tr("convert protocols"), this);
  tryConvertProtocolCheck->setChecked(true);
  tryConvertProtocolCheck->setToolTip(
      tr("Tries to convert the proprietary protocols of the imported commands "
          "to one of our own protocol types.\n"
          "Converted commands can be changed with the editor."));

  importButton = new QPushButton(tr("import selected device"), this);
  importButton->setEnabled(false);

  QHBoxLayout *manufacturerEditLayout = new QHBoxLayout();
  manufacturerEditLayout->addWidget(manufacturerEdit);
  manufacturerEditLayout->addWidget(clearManufacturerButton);

  QVBoxLayout *manufacturerLayout = new QVBoxLayout();
  manufacturerLayout->addLayout(manufacturerEditLayout);
  manufacturerLayout->addWidget(manufacturerList);

  QVBoxLayout *deviceLayout = new QVBoxLayout();
  deviceLayout->addWidget(deviceEdit);
  deviceLayout->addWidget(deviceList);

  QHBoxLayout *listsLayout = new QHBoxLayout();
  listsLayout->addLayout(manufacturerLayout);
  listsLayout->addLayout(deviceLayout);

  QHBoxLayout *importLayout = new QHBoxLayout();
  importLayout->addWidget(tryConvertProtocolCheck);
  importLayout->addStretch();
  importLayout->addWidget(importButton);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(listsLayout);
  mainLayout->addWidget(deviceInfoLabel);
  mainLayout->addLayout(importLayout);
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
  connect(importButton, &QPushButton::clicked, this,
      &DeviceSearchWidget::onImportClicked);
  connect(clearManufacturerButton, &QPushButton::clicked, this,
      &DeviceSearchWidget::onClearManufacturerClicked);
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
  const QString manufacturerName = getSelectedManufacturer();
  auto matches = QStringList();

  deviceList->clear();
  if (catalogue == nullptr) {
    return;
  }

  if (manufacturerName.isEmpty()) {
    matches = catalogue->findDevices(deviceEdit->text());
  } else {
    matches = catalogue->findDevices(manufacturerName, deviceEdit->text());
  }
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
  if (manufacturerName.isEmpty() && !deviceName.isEmpty()) {
    deviceInfoLabel->setText(tr("device name is used by several manufacturers, "
        "select a manufacturer"));
    return;
  }
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

void DeviceSearchWidget::updateImportButton()
{
  const bool hasSelection = (!getSelectedManufacturer().isEmpty())
      && (!getSelectedDevice().isEmpty());

  importButton->setEnabled((catalogue != nullptr) && hasSelection);
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

QString DeviceSearchWidget::findManufacturerOfDevice(
    const QString &deviceName) const
{
  QStringList manufacturers;
  QStringList matches;
  QString result;
  int count = 0;

  if (catalogue == nullptr) {
    return QString();
  }

  manufacturers = catalogue->findManufacturers(QString());
  for (const auto &manufacturerName : manufacturers) {
    matches = catalogue->findDevices(manufacturerName, deviceName);
    if (matches.contains(deviceName)) {
      result = manufacturerName;
      count++;
    }
  }

  if (count != 1) {
    return QString();
  }

  return result;
}

void DeviceSearchWidget::selectManufacturerOfDevice(const QString &deviceName)
{
  const QString manufacturerName = findManufacturerOfDevice(deviceName);
  QList<QListWidgetItem*> manufacturerItems;
  QList<QListWidgetItem*> deviceItems;

  if (manufacturerName.isEmpty()) {
    return;
  }

  manufacturerItems = manufacturerList->findItems(manufacturerName,
      Qt::MatchExactly);
  if (manufacturerItems.isEmpty()) {
    // the manufacturer is hidden by the manufacturer filter
    {
      const QSignalBlocker editBlocker(manufacturerEdit);
      manufacturerEdit->clear();
    }
    updateManufacturers();
    manufacturerItems = manufacturerList->findItems(manufacturerName,
        Qt::MatchExactly);
  }
  if (manufacturerItems.isEmpty()) {
    return;
  }

  {
    // rebuilding the device list must not trigger our own slots again
    const QSignalBlocker manufacturerBlocker(manufacturerList);
    const QSignalBlocker deviceBlocker(deviceList);

    manufacturerList->setCurrentItem(manufacturerItems.first());
    manufacturerList->scrollToItem(manufacturerItems.first());
    updateDevices();

    deviceItems = deviceList->findItems(deviceName, Qt::MatchExactly);
    if (!deviceItems.isEmpty()) {
      deviceList->setCurrentItem(deviceItems.first());
      deviceList->scrollToItem(deviceItems.first());
    }
  }
}

}
