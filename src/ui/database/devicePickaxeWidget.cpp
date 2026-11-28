// SPDX-License-Identifier: LGPL-2.1-or-later

#include "devicePickaxeWidget.h"

#include <algorithm>
#include <map>
#include <string>

#include <QAction>
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
#include <QSignalBlocker>
#include <QToolButton>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QEventLoop>

#include "lib/qtHelpers.h"
#include "lib/icon.h"
#include "document/data/enum.h"

using namespace std;

namespace database
{

using Database = document::files::Database;
using document::data::Enum;
using document::data::HardButtons;

DevicePickaxeWidget::DevicePickaxeWidget(Context &ctx,
    QAction *actionLearnIrSingle, QAction *actionLearnIrStream, QWidget *parent) :
    QWidget(parent), ctx(ctx), learnSingleAction(actionLearnIrSingle), learnStreamAction(
        actionLearnIrStream)
{
  createView();
  createConnections();
  updateButtonChoices();
  updateLearnControls();
  updateDeviceInfo();
  updatePickaxeButton();
  updateImportButton();
}

void DevicePickaxeWidget::setCatalogue(const Database *catalogue)
{
  // TODO: a running search still uses the old catalogue. the caller must not
  // delete it while searching (the widgets are disabled meanwhile)
  if (this->catalogue != nullptr) {
    disconnect(this->catalogue, &Database::pickaxeProgress, this,
        &DevicePickaxeWidget::onPickaxeProgress);
  }
  this->catalogue = catalogue;
  if (this->catalogue != nullptr) {
    connect(this->catalogue, &Database::pickaxeProgress, this,
        &DevicePickaxeWidget::onPickaxeProgress);
  }

  // the selection belongs to the old catalogue
  manufacturerList->clear();
  manufacturerEdit->clear();
  updateManufacturers();
  deviceEdit->clear();
  updateDevices();
  resultList->clear();
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

  //hide stream in simple mode. fixme -- user level changed slot
  for (const auto &button : learnStreamButton) {
    if (ctx.userLevel().getLevel() < lib::UserLevel::Level::Expert) {
      button->hide();
    } else {
      button->show();
    }
  }
}

void DevicePickaxeWidget::onLearnedCommand(
    ConcordConnection::LearnedCommandMode m, const binary::TimingStream &timing,
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
  const auto key = getSelectedResult();

  updateDeviceInfo();
  updateImportButton();

  if (!key.isEmpty()) {
    emit deviceSelected(key.manufacturerName, key.deviceName);
  }
}

void DevicePickaxeWidget::onButtonSelectionChanged(int row)
{
  learned[row].reset();
  updateButtonChoices();
  updateLearnControls();
  updatePickaxeButton();
}

void DevicePickaxeWidget::onLearnTriggered()
{
  if (searchRunning) {
    return;
  }

  learningRow = pendingRow;
  pendingRow = -1;
  if (learningRow < 0) {
    return;
  }

  learned[learningRow].reset();
  updateLearnControls();
  updatePickaxeButton();
}

void DevicePickaxeWidget::onPickaxeClicked()
{
  int count;
  const auto *db = catalogue;
  const auto devices = getDevicesToCheck();
  const auto *config = ctx.config();
  map<string, Database::LearnedCommand> commands;

  if ((db == nullptr) || devices.isEmpty()) {
    return;
  }
  if ((devices.size() == 1) && devices.first().isEmpty()) {
    // single manufacturer, but no device listed: an empty list would mean all
    emit writeMsg(tr("invalid selection"));
    return;
  }
  for (auto i = 0; i < COMMAND_COUNT; i++) {
    if (!learned[i].has_value()) {
      //should never get here
      return;
    }
    // key is the HardButtons string, "Power Toggle" is not part of the enum
    commands[buttonCombo[i]->currentText().toStdString()] = learned[i].value();
  }

  resultList->clear();
  count = 0;
  for (const QStringList &list : devices) {
    count = count + list.size();
  }
  progressBar->setRange(0, count);
  progressBar->setTextVisible(false);
  setSearchRunning(true);

  // blocking, returns when all devices are checked. the progress signal keeps
  // the ui alive meanwhile, see onPickaxeProgress()
  auto matches = db->pickaxe(devices, commands, *config);

  sort(matches.begin(), matches.end(), &DevicePickaxeWidget::isLessDeviceKey);
  for (const auto &key : matches) {
    addDeviceItem(resultList, key);
  }
  setSearchRunning(false);

  emit writeMsg(tr("pickaxe finished: %1 device(s) match").arg(matches.size()));
}

void DevicePickaxeWidget::onPickaxeProgress(int checked, int total)
{
  progressBar->setRange(0, total);
  progressBar->setValue(checked);

  // the search blocks this thread. give the ui an event pass now and then,
  // without user input: the widgets are locked anyway
  if (pickaxeLastEvents.elapsed() >= PICKAXE_EVENT_INTERVAL_MS) {
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    pickaxeLastEvents.restart();
  }
}

void DevicePickaxeWidget::onImportClicked()
{
  const auto key = getSelectedResult();
  const auto manufacturerName = key.manufacturerName;
  const auto deviceName = key.deviceName;
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
      tr("The pickaxe search finds the devices in the database that respond "
          "to the same three IR commands as your source remote. Steps:"
          "<ul>"
          "<li>connect your Harmony remote</li>"
          "<li>select one or more manufacturers</li>"
          "<li>optionally select the devices to check (single manufacturer "
          "only)</li>"
          "<li>learn three commands</li>"
          "<li>run the pickaxe search</li>"
          "<li>select a matching device and import it</li>"
          "</ul>"), this);
  explanationLabel->setTextFormat(Qt::RichText);
  explanationLabel->setWordWrap(true);

  hintLabel = new QLabel(this);
  hintLabel->setWordWrap(true);
  auto hintFont = hintLabel->font();
  hintFont.setBold(true);
  hintLabel->setFont(hintFont);

  manufacturerEdit = new QLineEdit(this);
  manufacturerEdit->setPlaceholderText(tr("search manufacturer"));

  clearManufacturerButton = new QPushButton(tr("clear"), this);
  clearManufacturerButton->setToolTip(tr("Deselect all manufacturers"));

  manufacturerList = new QListWidget(this);
  manufacturerList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  deviceEdit = new QLineEdit(this);
  deviceEdit->setPlaceholderText(tr("search device"));
  deviceEdit->setToolTip(
      tr("You can speed up the search by filtering and/or selecting "
          "devices here. Only what is selected will \nbe searched. When "
          "no manual selection is active, all currently visible devices "
          "are searched.\nOnly available "
          "with single manufacturer!."));

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

    learnSingleButton[i] = new QToolButton(this);
    learnSingleButton[i]->setDefaultAction(learnSingleAction);
    learnSingleButton[i]->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    learnStreamButton[i] = new QToolButton(this);
    learnStreamButton[i]->setDefaultAction(learnStreamAction);
    learnStreamButton[i]->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    learnStatus[i] = new QLabel(this);

    learnLayout->addWidget(buttonCombo[i], i, 0);
    learnLayout->addWidget(learnSingleButton[i], i, 1);
    learnLayout->addWidget(learnStreamButton[i], i, 2);
    learnLayout->addWidget(learnStatus[i], i, 3);
  }
  learnLayout->setColumnStretch(3, 1);

  const QIcon axeIcon = lib::getIcon(":/res/pickaxe.png");
  pickaxeButton = new QPushButton(axeIcon, tr(" start deep search"), this);
  pickaxeButton->setEnabled(false);

  progressBar = new QProgressBar(this);
  progressBar->setRange(0, 1);
  progressBar->setValue(0);
  progressBar->setFormat(tr("checked %v of %m devices"));
  progressBar->setTextVisible(false);

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
    connect(learnSingleButton[i], &QToolButton::pressed, this, [this, i]() {
      pendingRow = i;
    });
    connect(learnStreamButton[i], &QToolButton::pressed, this, [this, i]() {
      pendingRow = i;
    });
  }

  connect(learnSingleAction, &QAction::triggered, this,
      &DevicePickaxeWidget::onLearnTriggered);
  connect(learnStreamAction, &QAction::triggered, this,
      &DevicePickaxeWidget::onLearnTriggered);

  connect(pickaxeButton, &QPushButton::clicked, this,
      &DevicePickaxeWidget::onPickaxeClicked);
  connect(importButton, &QPushButton::clicked, this,
      &DevicePickaxeWidget::onImportClicked);
}

void DevicePickaxeWidget::updateManufacturers()
{
  const auto previous = getSelectedManufacturers();
  auto matches = QStringList();

  {
    // devices are updated once below, not for every item
    const QSignalBlocker blocker(manufacturerList);

    manufacturerList->clear();
    if (catalogue != nullptr) {
      matches = catalogue->findManufacturers(manufacturerEdit->text());
      matches.sort(Qt::CaseInsensitive);
      manufacturerList->addItems(matches);
    }

    // a changed filter keeps the selection of what is still visible
    for (auto i = 0; i < manufacturerList->count(); i++) {
      if (previous.contains(manufacturerList->item(i)->text())) {
        manufacturerList->item(i)->setSelected(true);
      }
    }
  }

  updateDevices();
}

void DevicePickaxeWidget::updateDevices()
{
  const auto manufacturerNames = getSelectedManufacturers();
  auto matches = QStringList();

  deviceList->clear();
  if ((catalogue != nullptr) && (manufacturerNames.size() == 1)) {
    matches = catalogue->findDevices(manufacturerNames.first(),
        deviceEdit->text());
    matches.sort(Qt::CaseInsensitive);
    deviceList->addItems(matches);
  }

  updateDeviceControls();
  updatePickaxeButton();
}

void DevicePickaxeWidget::updateDeviceControls()
{
  const auto manufacturerCount = getSelectedManufacturers().size();
  const auto selectable = (!searchRunning) && (manufacturerCount == 1);

  // device selection needs exactly one manufacturer
  deviceEdit->setEnabled(selectable);
  deviceList->setEnabled(selectable);
  if (manufacturerCount > 1) {
    deviceEdit->setPlaceholderText(
        tr("all devices of the selected manufacturers are checked"));
  } else {
    deviceEdit->setPlaceholderText(tr("search device"));
  }
}

void DevicePickaxeWidget::updateDeviceInfo()
{
  const auto key = getSelectedResult();

  deviceInfoLabel->clear();
  if ((catalogue == nullptr) || key.isEmpty()) {
    return;
  }

  const Database::Device info = catalogue->getDeviceInfo(key.manufacturerName,
      key.deviceName);
  if (info.name.isEmpty()) {
    deviceInfoLabel->setText(tr("device not found"));
    return;
  }

  deviceInfoLabel->setText(
      tr("%1 (Database type: %2)").arg(info.name).arg(info.type.getQString()));
}

void DevicePickaxeWidget::updateHint()
{
  const auto manufacturerCount = getSelectedManufacturers().size();
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
  } else if (manufacturerCount == 0) {
    hintLabel->setText(tr("next: select one or more manufacturers"));
  } else if (learningRow >= 0) {
    hintLabel->setText(tr("next: press the button on the remote"));
  } else if ((manufacturerCount == 1) && (deviceList->count() == 0)) {
    hintLabel->setText(tr("no device matches the filter"));
  } else if (learnedCount < COMMAND_COUNT) {
    hintLabel->setText(tr("next: learn the commands "
        "(%1 of %2 learned)").arg(learnedCount).arg(COMMAND_COUNT));
  } else if (resultList->count() > 0) {
    hintLabel->setText(tr("next: select a matching device "
        "to import it, or search again"));
  } else if (manufacturerCount > 1) {
    hintLabel->setText(tr("next: press search (all devices "
        "of %1 manufacturers will be "
        "checked)").arg(manufacturerCount));
  } else {
    hintLabel->setText(tr("next: press search (%1 device(s) "
        "will be checked)").arg(getDevicesToCheck().first().size()));
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

    if (learningRow == i) {
      learnStatus[i]->setText(tr("press the button on the remote..."));
    } else if (learned[i].has_value()) {
      learnStatus[i]->setText(
          tr("learned (%1 blocks, carrier %2 kHz)").arg(
              static_cast<int>(learned[i]->timing.timings().size())).arg(
              static_cast<double>(learned[i]->carrier) / 1000.0));
    } else {
      learnStatus[i]->setText(tr("not learned"));
    }
  }
}

void DevicePickaxeWidget::updatePickaxeButton()
{
  const auto manufacturerCount = getSelectedManufacturers().size();
  auto allLearned = true;

  for (auto i = 0; i < COMMAND_COUNT; i++) {
    if (!learned[i].has_value()) {
      allLearned = false;
    }
  }

  const bool ready = (catalogue != nullptr) && (manufacturerCount >= 1)
      && allLearned && !searchRunning && (learningRow < 0);
  pickaxeButton->setEnabled(ready);
  updateHint();
}

void DevicePickaxeWidget::updateImportButton()
{
  const bool ready = (catalogue != nullptr) && !searchRunning
      && !getSelectedResult().isEmpty();

  importButton->setEnabled(ready);
}

void DevicePickaxeWidget::setSearchRunning(bool running)
{
  searchRunning = running;

  manufacturerEdit->setEnabled(!running);
  clearManufacturerButton->setEnabled(!running);
  manufacturerList->setEnabled(!running);
  resultList->setEnabled(!running);

  updateDeviceControls();
  updateLearnControls();
  updatePickaxeButton();
  updateImportButton();
}

QStringList DevicePickaxeWidget::getSelectedManufacturers() const
{
  QStringList result;
  const auto items = manufacturerList->selectedItems();

  for (const auto *item : items) {
    result.append(item->text());
  }
  return result;
}

QMap<QString, QStringList> DevicePickaxeWidget::getDevicesToCheck() const
{
  QMap<QString, QStringList> result;
  const auto manufacturerNames = getSelectedManufacturers();
  const auto items = deviceList->selectedItems();

  for (const auto &manufacturerName : manufacturerNames) {
    result.insert(manufacturerName, QStringList());
  }

  if (manufacturerNames.size() != 1) {
    //all devices of all selected manufacturers
    for (const auto &mnf : result.keys()) {
      result[mnf] = catalogue->findDevices(mnf, "");
    }
    return result;
  }
  if (items.isEmpty()) {
    //all devices of single manufacturer
    for (auto i = 0; i < deviceList->count(); i++) {
      result[manufacturerNames.first()].append(deviceList->item(i)->text());
    }
    return result;
  }
  //selected devices of single manufacturer
  for (const auto *item : items) {
    result[manufacturerNames.first()].append(item->text());
  }
  return result;
}

Database::DeviceKey DevicePickaxeWidget::getSelectedResult() const
{
  const QList<QListWidgetItem*> items = resultList->selectedItems();
  if (items.isEmpty()) {
    return Database::DeviceKey();
  }

  return getDeviceKey(items.first());
}

void DevicePickaxeWidget::addDeviceItem(QListWidget *list,
    const Database::DeviceKey &key)
{
  // results can come from several manufacturers
  const auto text = QStringLiteral("%1 / %2").arg(key.manufacturerName,
      key.deviceName);
  auto *item = new QListWidgetItem(text, list);
  item->setData(Qt::UserRole, key.manufacturerName);
  item->setData(Qt::UserRole + 1, key.deviceName);
}

Database::DeviceKey DevicePickaxeWidget::getDeviceKey(
    const QListWidgetItem *item)
{
  Database::DeviceKey key;

  key.manufacturerName = item->data(Qt::UserRole).toString();
  key.deviceName = item->data(Qt::UserRole + 1).toString();
  return key;
}

bool DevicePickaxeWidget::isLessDeviceKey(const Database::DeviceKey &a,
    const Database::DeviceKey &b)
{
  const auto manufacturerOrder = a.manufacturerName.compare(b.manufacturerName,
      Qt::CaseInsensitive);

  if (manufacturerOrder != 0) {
    return manufacturerOrder < 0;
  }
  return a.deviceName.compare(b.deviceName, Qt::CaseInsensitive) < 0;
}

}
