// SPDX-License-Identifier: LGPL-2.1-or-later

#include "devicePickaxeWidget.h"

#include <algorithm>
#include <map>
#include <string>

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QtConcurrent>

#include "lib/qtHelpers.h"
#include "lib/icon.h"
#include "document/data/enum.h"

using namespace std;

namespace database
{

using Database = document::files::Database;
using document::data::Enum;
using document::data::HardButtons;

DevicePickaxeWidget::DevicePickaxeWidget(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx)
{
  createView();
  createConnections();
  updateButtonChoices();
  updateLearnControls();
  updateDeviceInfo();
  updatePickaxeButton();
  updateImportButton();
}

DevicePickaxeWidget::~DevicePickaxeWidget()
{
  // TODO: the search can't be cancelled yet, so we have to wait for it
  pickaxeWatcher->waitForFinished();
}

void DevicePickaxeWidget::setCatalogue(const Database *catalogue)
{
  if (this->catalogue == catalogue) {
    //lazy compare
    return;
  }

  // TODO: a running search still uses the old catalogue. the caller must not
  // delete it while searching (the widgets are disabled meanwhile)
  if (this->catalogue != nullptr) {
    disconnect(this->catalogue, &Database::pickaxeProgress, this,
        &DevicePickaxeWidget::onPickaxeProgress);
  }
  this->catalogue = catalogue;
  if (this->catalogue != nullptr) {
    // emitted from the worker thread, qt queues it into the gui thread
    connect(this->catalogue, &Database::pickaxeProgress, this,
        &DevicePickaxeWidget::onPickaxeProgress);
  }

  manufacturerEdit->clear();
  updateManufacturers();
  deviceEdit->clear();
  updateDevices();
  resultList->clear();
  resultManufacturer.clear();
  updateDeviceInfo();
  updatePickaxeButton();
  updateImportButton();
}

void DevicePickaxeWidget::setLearningAvailable(bool available)
{
  learningAvailable = available;
  if (!learningAvailable) {
    learningRow = -1;
  }

  // without a remote nothing can be done, the hint label says so
  contentWidget->setEnabled(learningAvailable);
  updateLearnControls();
  updatePickaxeButton();
}

void DevicePickaxeWidget::onLearnedCommand(const binary::TimingStream &timing,
    uint32_t carrier)
{
  Database::LearnedCommand command;

  if (learningRow < 0) {
    return;
  }
  if (timing.timings().empty()) {
    onLearnFailed(tr("no data received"));
    return;
  }

  command.timing = timing;
  command.carrier = carrier;
  learned[learningRow] = command;
  learningRow = -1;
  updateLearnControls();
  updatePickaxeButton();
}

void DevicePickaxeWidget::onLearnFailed(const QString &message)
{
  if (learningRow < 0) {
    return;
  }

  learningRow = -1;
  emit writeMsg(tr("learning failed: %1").arg(message));
  updateLearnControls();
  updatePickaxeButton();
}

void DevicePickaxeWidget::onManufacturerTextChanged(const QString &text)
{
  updateManufacturers();
}

void DevicePickaxeWidget::onManufacturerSelectionChanged()
{
  updateDevices();
}

void DevicePickaxeWidget::onDeviceTextChanged(const QString &text)
{
  updateDevices();
}

void DevicePickaxeWidget::onDeviceSelectionChanged()
{
  // the number of devices to check changes, shown in the hint
  updatePickaxeButton();
}

void DevicePickaxeWidget::onResultSelectionChanged()
{
  const auto deviceName = getSelectedResult();

  updateDeviceInfo();
  updateImportButton();

  if (!resultManufacturer.isEmpty() && !deviceName.isEmpty()) {
    emit deviceSelected(resultManufacturer, deviceName);
  }
}

void DevicePickaxeWidget::onButtonSelectionChanged(int row)
{
  learned[row].reset();
  updateButtonChoices();
  updateLearnControls();
  updatePickaxeButton();
}

void DevicePickaxeWidget::onLearnClicked(int row)
{
  if (!learningAvailable || searchRunning || (learningRow >= 0)) {
    return;
  }

  learned[row].reset();
  learningRow = row;
  updateLearnControls();
  updatePickaxeButton();

  emit requestLearn();
}

void DevicePickaxeWidget::onPickaxeClicked()
{
  const auto db = catalogue;
  const auto manufacturerName = getSelectedManufacturer();
  const auto devices = getDevicesToCheck();
  const auto deviceCount = static_cast<int>(devices.size());
  map<string, Database::LearnedCommand> commands;

  if ((db == nullptr) || manufacturerName.isEmpty() || devices.isEmpty()) {
    return;
  }
  for (auto i = 0; i < COMMAND_COUNT; i++) {
    if (!learned[i].has_value()) {
      return;
    }
    // key is the HardButtons string, "Power Toggle" is not part of the enum
    commands[buttonCombo[i]->currentText().toStdString()] = learned[i].value();
  }

  resultList->clear();
  resultManufacturer = manufacturerName;
  progressBar->setRange(0, deviceCount);
  progressBar->setValue(0);
  progressBar->setTextVisible(true);
  setSearchRunning(true);

  // blocking call, so it runs in a worker thread. result: onPickaxeFinished
  pickaxeWatcher->setFuture(QtConcurrent::run(
      [db, manufacturerName, devices, commands]() {
        return db->pickaxe(manufacturerName, devices, commands);
      }));
}

void DevicePickaxeWidget::onPickaxeProgress(int checked, int total)
{
  progressBar->setRange(0, total);
  progressBar->setValue(checked);
}

void DevicePickaxeWidget::onPickaxeFinished()
{
  QStringList names;
  const auto matches = pickaxeWatcher->result();
  const auto total = progressBar->maximum();

  for (const auto &device : matches) {
    names.append(device.name);
  }
  names.sort(Qt::CaseInsensitive);

  resultList->addItems(names);
  progressBar->setValue(total);
  setSearchRunning(false);
  updateDeviceInfo();

  emit writeMsg(
      tr("pickaxe finished: %1 of %2 device(s) match").arg(matches.size()).arg(
          total));
}

void DevicePickaxeWidget::onImportClicked()
{
  const auto manufacturerName = resultManufacturer;
  const auto deviceName = getSelectedResult();
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

void DevicePickaxeWidget::onClearManufacturerClicked()
{
  manufacturerList->clearSelection();
  manufacturerEdit->clear();
}

void DevicePickaxeWidget::createView()
{
  QStringList buttonNames;

  // power toggle has no enum value, the other defaults come from HardButtons
  const array<QString, COMMAND_COUNT> defaultButtons = {
    POWER_BUTTON,
    Enum<HardButtons>::toQString(HardButtons::Select),
    Enum<HardButtons>::toQString(HardButtons::Number3) };
  const auto hardButtons = Enum<HardButtons>::toStringList();

  // power toggle is not part of HardButtons, so it is added here
  buttonNames.append(POWER_BUTTON);
  for (const auto &name : hardButtons) {
    if (name != Enum<HardButtons>::toString(HardButtons::Unknown)) {
      buttonNames.append(qstr(name));
    }
  }

  explanationLabel = new QLabel(
      tr("Finds the devices in the database that respond to the same three "
          "IR commands as your source remote. Connect your Harmony, select a "
          "manufacturer, optionally select the devices to check, learn three "
          "commands, run the pickaxe search, then select a matching device and "
          "import it."), this);
  explanationLabel->setWordWrap(true);

  hintLabel = new QLabel(this);
  hintLabel->setWordWrap(true);
  auto hintFont = hintLabel->font();
  hintFont.setBold(true);
  hintLabel->setFont(hintFont);

  manufacturerEdit = new QLineEdit(this);
  manufacturerEdit->setPlaceholderText(tr("search manufacturer"));

  clearManufacturerButton = new QPushButton(tr("clear"), this);
  clearManufacturerButton->setToolTip(tr("Deselect the manufacturer"));

  manufacturerList = new QListWidget(this);
  manufacturerList->setSelectionMode(QAbstractItemView::SingleSelection);

  deviceEdit = new QLineEdit(this);
  deviceEdit->setPlaceholderText(tr("search device"));
  deviceEdit->setToolTip(
      tr("Selected devices are checked. Without a selection all listed "
          "devices are checked."));

  deviceList = new QListWidget(this);
  deviceList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto *learnBox = new QGroupBox(tr("commands to match"), this);
  auto *learnLayout = new QGridLayout(learnBox);
  for (auto i = 0; i < COMMAND_COUNT; i++) {
    buttonCombo[i] = new QComboBox(this);
    buttonCombo[i]->addItems(buttonNames);
    buttonCombo[i]->setCurrentIndex(
        max(0, buttonCombo[i]->findText(defaultButtons[i])));
    buttonCombo[i]->setToolTip(tr("The button you will press on the remote"));

    learnButton[i] = new QPushButton(tr("learn"), this);
    learnButton[i]->setEnabled(false);

    learnStatus[i] = new QLabel(this);

    learnLayout->addWidget(buttonCombo[i], i, 0);
    learnLayout->addWidget(learnButton[i], i, 1);
    learnLayout->addWidget(learnStatus[i], i, 2);
  }
  learnLayout->setColumnStretch(2, 1);

  const QIcon axeIcon = lib::getIcon(":/res/pickaxe.png");
  pickaxeButton = new QPushButton(axeIcon, tr(" start deep search"), this);
  pickaxeButton->setEnabled(false);

  progressBar = new QProgressBar(this);
  progressBar->setRange(0, 1);
  progressBar->setValue(0);
  progressBar->setFormat(tr("checked %v of %m devices"));
  progressBar->setTextVisible(false);

  pickaxeWatcher = new QFutureWatcher<QList<Database::Device>>(this);

  resultList = new QListWidget(this);
  resultList->setSelectionMode(QAbstractItemView::SingleSelection);

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

  auto *manufacturerEditLayout = new QHBoxLayout();
  manufacturerEditLayout->addWidget(manufacturerEdit);
  manufacturerEditLayout->addWidget(clearManufacturerButton);

  auto *manufacturerLayout = new QVBoxLayout();
  manufacturerLayout->addLayout(manufacturerEditLayout);
  manufacturerLayout->addWidget(manufacturerList);

  auto *deviceLayout = new QVBoxLayout();
  deviceLayout->addWidget(deviceEdit);
  deviceLayout->addWidget(deviceList);

  auto *listsLayout = new QHBoxLayout();
  listsLayout->addLayout(manufacturerLayout);
  listsLayout->addLayout(deviceLayout);

  auto *pickaxeLayout = new QHBoxLayout();
  pickaxeLayout->addWidget(pickaxeButton);
  pickaxeLayout->addWidget(progressBar, 1);

  auto *importLayout = new QHBoxLayout();
  importLayout->addWidget(tryConvertProtocolCheck);
  importLayout->addStretch();
  importLayout->addWidget(importButton);

  // workflow runs top to bottom, results are the last step
  auto *resultBox = new QGroupBox(tr("matching devices"), this);
  auto *resultLayout = new QVBoxLayout(resultBox);
  resultLayout->addWidget(resultList);
  resultLayout->addWidget(deviceInfoLabel);
  resultLayout->addLayout(importLayout);

  // everything below the hint is locked until the remote is connected
  contentWidget = new QWidget(this);
  contentWidget->setEnabled(false);
  auto *contentLayout = new QVBoxLayout(contentWidget);
  contentLayout->setContentsMargins(0, 0, 0, 0);
  contentLayout->addLayout(listsLayout, 3);
  contentLayout->addWidget(learnBox);
  contentLayout->addLayout(pickaxeLayout);
  contentLayout->addWidget(resultBox, 2);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(explanationLabel);
  mainLayout->addWidget(hintLabel);
  mainLayout->addWidget(contentWidget, 1);
}

void DevicePickaxeWidget::createConnections()
{
  connect(manufacturerEdit, &QLineEdit::textChanged, this,
      &DevicePickaxeWidget::onManufacturerTextChanged);
  connect(manufacturerList, &QListWidget::itemSelectionChanged, this,
      &DevicePickaxeWidget::onManufacturerSelectionChanged);
  connect(clearManufacturerButton, &QPushButton::clicked, this,
      &DevicePickaxeWidget::onClearManufacturerClicked);
  connect(deviceEdit, &QLineEdit::textChanged, this,
      &DevicePickaxeWidget::onDeviceTextChanged);
  connect(deviceList, &QListWidget::itemSelectionChanged, this,
      &DevicePickaxeWidget::onDeviceSelectionChanged);
  connect(resultList, &QListWidget::itemSelectionChanged, this,
      &DevicePickaxeWidget::onResultSelectionChanged);

  for (auto i = 0; i < COMMAND_COUNT; i++) {
    connect(buttonCombo[i], QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this, i]() {
          onButtonSelectionChanged(i);
        });
    connect(learnButton[i], &QPushButton::clicked, this, [this, i]() {
      onLearnClicked(i);
    });
  }

  connect(pickaxeButton, &QPushButton::clicked, this,
      &DevicePickaxeWidget::onPickaxeClicked);
  connect(pickaxeWatcher, &QFutureWatcherBase::finished, this,
      &DevicePickaxeWidget::onPickaxeFinished);
  connect(importButton, &QPushButton::clicked, this,
      &DevicePickaxeWidget::onImportClicked);
}

void DevicePickaxeWidget::updateManufacturers()
{
  manufacturerList->clear();
  if (catalogue == nullptr) {
    return;
  }

  auto matches = catalogue->findManufacturers(manufacturerEdit->text());
  matches.sort(Qt::CaseInsensitive);
  manufacturerList->addItems(matches);
}

void DevicePickaxeWidget::updateDevices()
{
  const QString manufacturerName = getSelectedManufacturer();
  auto matches = QStringList();

  deviceList->clear();
  if ((catalogue != nullptr) && !manufacturerName.isEmpty()) {
    matches = catalogue->findDevices(manufacturerName, deviceEdit->text());
    matches.sort(Qt::CaseInsensitive);
    deviceList->addItems(matches);
  }

  updatePickaxeButton();
}

void DevicePickaxeWidget::updateDeviceInfo()
{
  const QString resultName = getSelectedResult();

  deviceInfoLabel->clear();
  if ((catalogue == nullptr) || resultName.isEmpty()
      || resultManufacturer.isEmpty()) {
    return;
  }

  const Database::Device info = catalogue->getDeviceInfo(resultManufacturer,
      resultName);
  if (info.name.isEmpty()) {
    deviceInfoLabel->setText(tr("device not found"));
    return;
  }

  deviceInfoLabel->setText(
      tr("%1 (Database type: %2)").arg(info.name).arg(info.type.getQString()));
}

void DevicePickaxeWidget::updateHint()
{
  auto learnedCount = 0;

  for (auto i = 0; i < COMMAND_COUNT; i++) {
    if (learned[i].has_value()) {
      learnedCount++;
    }
  }

  // tells the user what to do next, the workflow order is fixed except for
  // manufacturer/devices vs. learning, which can be done in any order
  if (searchRunning) {
    hintLabel->setText(tr("searching..."));
  } else if (catalogue == nullptr) {
    hintLabel->setText(tr("No devices available!"));
  } else if (!learningAvailable) {
    hintLabel->setText(tr("next: connect your harmony remote"));
  } else if (getSelectedManufacturer().isEmpty()) {
    hintLabel->setText(tr("next: select a manufacturer"));
  } else if (learningRow >= 0) {
    hintLabel->setText(tr("next: press the button on the remote"));
  } else if (deviceList->count() == 0) {
    hintLabel->setText(tr("no device matches the filter"));
  } else if (learnedCount < COMMAND_COUNT) {
    hintLabel->setText(
        tr("next: learn the commands (%1 of %2 learned)").arg(learnedCount).arg(
            COMMAND_COUNT));
  } else if (resultList->count() > 0) {
    hintLabel->setText(
        tr("next: select a matching device to import it, or search again"));
  } else {
    hintLabel->setText(
        tr("next: press search (%1 device(s) will be checked)").arg(
            getDevicesToCheck().size()));
  }
}

void DevicePickaxeWidget::updateButtonChoices()
{
  for (auto i = 0; i < COMMAND_COUNT; i++) {
    auto *model = qobject_cast<QStandardItemModel*>(buttonCombo[i]->model());
    if (model == nullptr) {
      continue;
    }

    // every button type can be used once: disable what the other rows use
    for (auto item = 0; item < model->rowCount(); item++) {
      const auto text = buttonCombo[i]->itemText(item);
      auto usedElsewhere = false;

      for (auto other = 0; other < COMMAND_COUNT; other++) {
        if ((other != i) && (buttonCombo[other]->currentText() == text)) {
          usedElsewhere = true;
        }
      }
      model->item(item)->setEnabled(!usedElsewhere);
    }
  }
}

void DevicePickaxeWidget::updateLearnControls()
{
  const auto idle = (!searchRunning) && (learningRow < 0);

  for (auto i = 0; i < COMMAND_COUNT; i++) {
    buttonCombo[i]->setEnabled(idle);
    learnButton[i]->setEnabled(idle && learningAvailable);

    if (learningRow == i) {
      learnStatus[i]->setText(tr("press the button on the remote..."));
    } else if (learned[i].has_value()) {
      learnStatus[i]->setText(
          tr("learned (%1 blocks, carrier %2)").arg(
              static_cast<int>(learned[i]->timing.timings().size())).arg(
              learned[i]->carrier));
    } else {
      learnStatus[i]->setText(tr("not learned"));
    }
  }
}

void DevicePickaxeWidget::updatePickaxeButton()
{
  auto allLearned = true;

  for (auto i = 0; i < COMMAND_COUNT; i++) {
    if (!learned[i].has_value()) {
      allLearned = false;
    }
  }

  const bool ready = (catalogue != nullptr)
      && !getSelectedManufacturer().isEmpty() && (deviceList->count() > 0)
      && allLearned && !searchRunning && (learningRow < 0);

  pickaxeButton->setEnabled(ready);
  updateHint();
}

void DevicePickaxeWidget::updateImportButton()
{
  const bool ready = (catalogue != nullptr) && !searchRunning
      && !resultManufacturer.isEmpty() && !getSelectedResult().isEmpty();

  importButton->setEnabled(ready);
}

void DevicePickaxeWidget::setSearchRunning(bool running)
{
  searchRunning = running;

  manufacturerEdit->setEnabled(!running);
  clearManufacturerButton->setEnabled(!running);
  manufacturerList->setEnabled(!running);
  deviceEdit->setEnabled(!running);
  deviceList->setEnabled(!running);
  resultList->setEnabled(!running);

  updateLearnControls();
  updatePickaxeButton();
  updateImportButton();
}

QString DevicePickaxeWidget::getSelectedManufacturer() const
{
  const QList<QListWidgetItem*> items = manufacturerList->selectedItems();
  if (items.isEmpty()) {
    return QString();
  }

  return items.first()->text();
}

QStringList DevicePickaxeWidget::getDevicesToCheck() const
{
  const QList<QListWidgetItem*> items = deviceList->selectedItems();
  QStringList result;

  if (items.isEmpty()) {
    // no selection: everything the filter lets through
    for (auto i = 0; i < deviceList->count(); i++) {
      result.append(deviceList->item(i)->text());
    }
  } else {
    for (const auto *item : items) {
      result.append(item->text());
    }
  }

  return result;
}

QString DevicePickaxeWidget::getSelectedResult() const
{
  const QList<QListWidgetItem*> items = resultList->selectedItems();
  if (items.isEmpty()) {
    return QString();
  }

  return items.first()->text();
}

}
