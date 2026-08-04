// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "buttonListModel.h"

using namespace std;

namespace models
{

ButtonBaseModel::ButtonBaseModel(document::Config &config,
    document::data::Item item, QObject *parent) :
    QAbstractItemModel(parent), config(config), item(item)
{
  createActions();
}

ButtonBaseModel::~ButtonBaseModel() = default;

QVariant ButtonBaseModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
  if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
    return {};
  }
  try {
    return columnSetup.at(mapColumn(section)).name;
  } catch (...) {
  }
  return {};
}

QVariant ButtonBaseModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return {};
  }
  if (index.parent().isValid()) {
    return {};
  }

  switch (role) {
    case Qt::DisplayRole:
      return getDisplayData(index);
    case Qt::EditRole:
      return getEditData(index);
    case Qt::ToolTipRole:
      return getTooltipData(index);
    case Qt::BackgroundRole:
      return getBackgroundData(index);
    case Qt::ForegroundRole:
      return getForegroundData(index);
    case UserDataRole::SelectionItemsRole:
      return getSelectionItemsData(index);
    default:
      return {};
  }
}

QModelIndex ButtonBaseModel::index(int row, int column,
    const QModelIndex &parent) const
{
  if (parent.isValid()) {
    //not a hierarchical model! parent = header, data = first.
    return {};
  }
  if ((row >= getButtons().size()) || (mapColumn(column) >= Column::COUNT)) {
    return {};
  }
  return createIndex(row, column, &getButtons()[row]); //no "mapColumn()"!
}

QModelIndex ButtonBaseModel::parent(const QModelIndex &index) const
{
  return {};
}

int ButtonBaseModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return getButtons().size();
}

Qt::ItemFlags ButtonBaseModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  try {
    auto isConst = columnSetup.at(mapColumn(index.column())).isConst;
    if (!isConst) {
      return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
  } catch (...) {
  }
  return QAbstractItemModel::flags(index);
}

bool ButtonBaseModel::setHeaderData(int section, Qt::Orientation orientation,
    const QVariant &value, int role)
{
  return false;
}

bool ButtonBaseModel::insertColumns(int position, int columns,
    const QModelIndex &parent)
{
  return false;
}

bool ButtonBaseModel::removeColumns(int position, int columns,
    const QModelIndex &parent)
{
  return false;
}

bool ButtonBaseModel::insertRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }

  //no begin/end insert rows, is done inside observers

  config.beginMacro(QObject::tr("Add %1 buttons(s)").arg(rows));

  for (int i = position; i < position + rows; i++) {
    auto ret = addButton(i);
    if (!ret) {
      success = false;
      continue;
    }
  }

  //todo fill the row with data that is not plainly invalid

  config.endMacro();

  return success;
}

bool ButtonBaseModel::removeRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }
  auto &buttons = getButtons();
  if ((position + rows) > buttons.size()) {
    return false;
  }

  //no begin/end remove rows, is done inside observers

  config.beginMacro(QObject::tr("Remove %1 buttons(s)").arg(rows));

  for (int i = 0; i < rows; i++) {
    //remove beginning at last item
    auto currRow = position + rows - 1 - i;
    auto ret = removeButton(currRow);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

void models::ButtonBaseModel::createActions()
{
  connect(&config, &document::Config::itemChanged, this,
      &ButtonBaseModel::itemChangedObserver);
  connect(&config, &document::Config::itemAboutToBeAdded, this,
      &ButtonBaseModel::itemAboutToBeAddedObserver);
  connect(&config, &document::Config::itemAdded, this,
      &ButtonBaseModel::itemAddedObserver);
  connect(&config, &document::Config::itemAboutToBeRemoved, this,
      &ButtonBaseModel::itemAboutToBeRemovedObserver);
  connect(&config, &document::Config::itemRemoved, this,
      &ButtonBaseModel::itemRemovedObserver);
}

QStringList ButtonBaseModel::getUnusedButtons() const
{
  auto list =
      document::data::Enum<document::data::HardButtons>::toQStringList();
  for (const auto &button : getButtons()) {
    auto name = QString::fromStdString(button.name.get());
    list.removeAll(name);
  }
  return list;
}

QStringList ButtonBaseModel::getUnusedButtons(
    const document::data::item::Button &button) const
{
  auto list = getUnusedButtons();
  auto current = QString::fromStdString(button.name.get());

  if (!list.contains(current)) {
    list.append(current);
  }

  return list;
}

QStringList ButtonBaseModel::getUnusedPositions() const
{
  //only allow each position once. always add a new, empty
  //page once a page has at least one button on it.
  int i;
  set<int> positions;
  QStringList list;
  int buttonCount = SOFTBUTTONS_PER_PAGE; //min one page

  //get all positions
  for (const auto &button : getButtons()) {
    auto pos = button.position.get();
    if (pos >= 0) {
      positions.insert(pos);
    }
  }
  //find the button with the highest index
  if (!positions.empty()) {
    auto last = *positions.rbegin();
    auto pages = (last + SOFTBUTTONS_PER_PAGE - 1) / SOFTBUTTONS_PER_PAGE + 1;
    buttonCount = (pages + 1) * SOFTBUTTONS_PER_PAGE; //add one empty page
  }
  //create entries for all others (and current entry)
  for (i = 0; i < buttonCount; i++) {
    if (positions.contains(i)) {
      continue;
    }
    list.push_back(toPositionString(i));
  }
  return list;
}

QStringList ButtonBaseModel::getUnusedPositions(
    const document::data::item::Button &button) const
{
  auto list = getUnusedPositions();
  auto current = toPositionString(button.position.get());

  if (!list.contains(current)) {
    list.prepend(current);
  }
  return list;
}

QString models::ButtonBaseModel::toPositionString(int pos) const
{
  return tr("Position: %1 (Page %2, Button %3)").arg(pos).arg(
      pos / SOFTBUTTONS_PER_PAGE + 1).arg(pos % SOFTBUTTONS_PER_PAGE + 1);
}

int models::ButtonBaseModel::fromPositionString(const QString &pos) const
{
  return pos.section(" ", 1, 1).toInt();
}

QStringList ButtonBaseModel::getAvailableCommands(
    const document::data::item::Device *device) const
{
  QStringList commands;

  for (const auto &irCommand : device->getIrCommands().getRawCommands()) {
    commands.push_back(QString::fromStdString(irCommand.name.get()));
  }
  for (const auto &irCommand : device->getIrCommands().getProtoCommands()) {
    commands.push_back(QString::fromStdString(irCommand.name.get()));
  }
  commands.sort();
  return commands;
}

void ButtonBaseModel::itemChangedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  //we don't know the column!
  emit dataChanged(index(pos, 0), index(pos, columnCount()), {
    Qt::DisplayRole,
    Qt::EditRole });
}

void ButtonBaseModel::itemAboutToBeAddedObserver(document::data::Item item,
    int pos)
{
  if (item != this->item) {
    return;
  }
  emit beginInsertRows(QModelIndex(), pos, pos);
}

void ButtonBaseModel::itemAddedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit endInsertRows();
}

void ButtonBaseModel::itemAboutToBeRemovedObserver(document::data::Item item,
    int pos)
{
  if (item != this->item) {
    return;
  }
  emit beginRemoveRows(QModelIndex(), pos, pos);
}

void ButtonBaseModel::itemRemovedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit endRemoveRows();
}

DeviceHardButtonModel::DeviceHardButtonModel(document::Config &config,
    uint32_t deviceId, QObject *parent) :
    ButtonBaseModel(config, document::data::Item::DEVICE_HARD_BUTTON)
{
  device = config.data().getDevice(deviceId, &devicePos);
}

bool DeviceHardButtonModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto baseColumn = mapColumn(index.column());

  //checks
  if (!columnSetup.contains(baseColumn)) {
    return false;
  }
  if (columnSetup.at(baseColumn).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  switch (static_cast<Column>(index.column())) {
    case Column::COMMAND:
      return config.modify().setDeviceButtonAction(
          value.toString().toStdString(), devicePos, type, row);
    case Column::BUTTON:
      return config.modify().setDeviceButtonName(value.toString().toStdString(),
          devicePos, type, row);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

ButtonBaseModel::Column models::DeviceHardButtonModel::mapColumn(
    int viewColumn) const
{
  return mapColumn(static_cast<Column>(viewColumn));
}

ButtonBaseModel::Column models::DeviceHardButtonModel::mapColumn(
    Column viewColumn) const
{
  switch (viewColumn) {
    case Column::COMMAND:
      return ButtonBaseModel::Column::COMMAND;
    case Column::BUTTON:
      return ButtonBaseModel::Column::BUTTON;
    default:
      return ButtonBaseModel::Column::COUNT;
  }
}

int DeviceHardButtonModel::columnCount(const QModelIndex &parent) const
{
  return static_cast<int>(Column::COUNT);
}

const vector<document::data::item::Button>& DeviceHardButtonModel::getButtons() const
{
  return device->getHardButtons();
}

bool DeviceHardButtonModel::addButton(int row, bool setDefaults)
{
  QString commandToUse;
  auto commands = getAvailableCommands(device);
  auto buttons = getUnusedButtons();

  if (commands.size() == 0) {
    emit writeMsg(
        tr("No IR commands available to add. Add an IR command first"));
    return false;
  }
  if (buttons.size() == 0) {
    emit writeMsg(tr("All hardware buttons are used. Can't add more"));
    return false;
  }
  auto buttonToUse = buttons[0];

  auto ret = config.modify().addDeviceButtonCommand(devicePos, type, row);
  if (ret != true) {
    return ret;
  }
  if (commands.contains(buttonToUse)) {
    commandToUse = buttonToUse;
  } else {
    commandToUse = commands[0];
  }
  config.modify().setDeviceButtonAction(commandToUse.toStdString(), devicePos,
      type, row);
  config.modify().setDeviceButtonName(buttonToUse.toStdString(), devicePos,
      type, row);
  return true;
}

bool DeviceHardButtonModel::removeButton(int row)
{
  return config.modify().removeDeviceButtonCommand(devicePos, type, row);
}

QVariant DeviceHardButtonModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::BUTTON:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceHardButtonModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::BUTTON:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceHardButtonModel::getTooltipData(const QModelIndex &index) const
{
  try {
    auto text = columnSetup.at(mapColumn(index.column())).context;
    switch (static_cast<Column>(index.column())) {
      case Column::BUTTON: {
        auto name = QString::fromStdString(
            getButtons().at(index.row()).name.get());
        text = tr("<b>%1</b><br>"
            "You can find it here:<br><br>"
            "<img src=':/res/buttons/%2.jpg' width='180' height='138'>").arg(
            text).arg(name);
        return text;
      }
      default:
        return text;
    }
  } catch (...) {
  }
  return {};
}

QVariant DeviceHardButtonModel::getBackgroundData(
    const QModelIndex &index) const
{
  return {};
}

QVariant DeviceHardButtonModel::getForegroundData(
    const QModelIndex &index) const
{
  return {};
}

QVariant DeviceHardButtonModel::getSelectionItemsData(
    const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return getAvailableCommands(device);
      case Column::BUTTON:
        return getUnusedButtons(button);
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

DeviceSoftButtonModel::DeviceSoftButtonModel(document::Config &config,
    uint32_t deviceId, QObject *parent) :
    ButtonBaseModel(config, document::data::Item::DEVICE_SOFT_BUTTON)
{
  device = config.data().getDevice(deviceId, &devicePos);
}

bool DeviceSoftButtonModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto baseColumn = mapColumn(index.column());

  //checks
  if (!columnSetup.contains(baseColumn)) {
    return false;
  }
  if (columnSetup.at(baseColumn).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  switch (static_cast<Column>(index.column())) {
    case Column::COMMAND:
      return config.modify().setDeviceButtonAction(
          value.toString().toStdString(), devicePos, type, row);
    case Column::NAME:
      return config.modify().setDeviceButtonName(value.toString().toStdString(),
          devicePos, type, row);
    case Column::POSITION: {
      auto pos = fromPositionString(value.toString());
      return config.modify().setDeviceButtonPosition(pos, devicePos, type, row);
    }
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

ButtonBaseModel::Column models::DeviceSoftButtonModel::mapColumn(
    int viewColumn) const
{
  return mapColumn(static_cast<Column>(viewColumn));
}

ButtonBaseModel::Column models::DeviceSoftButtonModel::mapColumn(
    Column viewColumn) const
{
  switch (viewColumn) {
    case Column::COMMAND:
      return ButtonBaseModel::Column::COMMAND;
    case Column::NAME:
      return ButtonBaseModel::Column::NAME;
    case Column::POSITION:
      return ButtonBaseModel::Column::POSITION;
    default:
      return ButtonBaseModel::Column::COUNT;
  }
}

int DeviceSoftButtonModel::columnCount(const QModelIndex &parent) const
{
  return static_cast<int>(Column::COUNT);
}

const vector<document::data::item::Button>& DeviceSoftButtonModel::getButtons() const
{
  return device->getSoftButtons();
}

bool DeviceSoftButtonModel::addButton(int row, bool setDefaults)
{
  auto commands = getAvailableCommands(device);
  if (commands.size() == 0) {
    emit writeMsg(
        tr("No IR commands available to add. Add an IR command first"));
    return false;
  }

  auto ret = config.modify().addDeviceButtonCommand(devicePos, type, row);
  if (ret != true) {
    return ret;
  }
  //for default -- first unused command.
  auto unusedCommands = getUnusedCommands(device);
  if (unusedCommands.size() == 0) {
    unusedCommands = commands;
  }
  config.modify().setDeviceButtonAction(unusedCommands[0].toStdString(),
      devicePos, type, row);
  config.modify().setDeviceButtonName(unusedCommands[0].toStdString(),
      devicePos, type, row);
  //assume infinite touch positions for the real world
  auto position = fromPositionString(getUnusedPositions()[0]);
  config.modify().setDeviceButtonPosition(position, devicePos, type, row);
  return true;
}

bool DeviceSoftButtonModel::removeButton(int row)
{
  return config.modify().removeDeviceButtonCommand(devicePos, type, row);
}

QVariant DeviceSoftButtonModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::NAME:
        return QString::fromStdString(button.name.get());
      case Column::POSITION:
        return toPositionString(button.position.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceSoftButtonModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::NAME:
        return QString::fromStdString(button.name.get());
      case Column::POSITION:
        return toPositionString(button.position.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceSoftButtonModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(mapColumn(index.column())).context;
  } catch (...) {
  }
  return {};
}

QVariant DeviceSoftButtonModel::getBackgroundData(
    const QModelIndex &index) const
{
  return {};
}

QVariant DeviceSoftButtonModel::getForegroundData(
    const QModelIndex &index) const
{
  return {};
}

QVariant DeviceSoftButtonModel::getSelectionItemsData(
    const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return getAvailableCommands(device);
      case Column::POSITION:
        return getUnusedPositions(button);
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QStringList models::DeviceSoftButtonModel::getUnusedCommands(
    const document::data::item::Device *device) const
{
  QStringList usedCommands;

  for (const auto &button : device->getHardButtons()) {
    usedCommands.push_back(QString::fromStdString(button.action.get()));
  }
  for (const auto &button : device->getSoftButtons()) {
    usedCommands.push_back(QString::fromStdString(button.action.get()));
  }
  auto availableCommands = getAvailableCommands(device);

  for (const auto &command : usedCommands) {
    availableCommands.removeAll(command);
  }
  return availableCommands;
}

}

