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
    ButtonBaseModel(config, document::data::Item::DEVICE_BUTTON)
{
  device = config.data().getDevice(deviceId, &devicePos);
}

QVariant DeviceHardButtonModel::data(const QModelIndex &index, int role) const
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
          value.toString().toStdString(), devicePos, row);
    case Column::BUTTON:
      return config.modify().setDeviceButtonName(value.toString().toStdString(),
          devicePos, row);
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
  return device->getButtons();
}

bool DeviceHardButtonModel::addButton(int row)
{
  return config.modify().addDeviceButtonCommand(
      document::data::item::ButtonType::Hard, devicePos, row);
}

bool DeviceHardButtonModel::removeButton(int row)
{
  return config.modify().removeDeviceButtonCommand(devicePos, row);
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
    return columnSetup.at(mapColumn(index.column())).context;
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
        return getSelectionItemsDataCommand(button);
      case Column::BUTTON:
        return getSelectionItemsDataButton(button);
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceHardButtonModel::getSelectionItemsDataCommand(
    const document::data::item::Button &button) const
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

QVariant DeviceHardButtonModel::getSelectionItemsDataButton(
    const document::data::item::Button &button) const
{
  //only allow unused buttons
  auto current = QString::fromStdString(button.name.get());
  auto list =
      document::data::Enum<document::data::HardButtons>(current).getQStringList();
  for (const auto &button : getButtons()) {
    auto name = QString::fromStdString(button.name.get());
    if (name == current) {
      continue;
    }
    list.removeAll(name);
  }
  return list;
}

//
//
//QVariant DeviceHardButtonModel::getDisplayData(const QModelIndex &index) const
//{
//  try {
//    auto &button = getButtons().at(index.row());
//    switch (index.column()) {
//      case Column::DEVICE: {
//        auto id = button.device.get();
//        auto *device = config.data().getDevice(id);
//        if (device != nullptr) {
//          return QString::fromStdString(device->label.get());
//        }
//        return {};
//      }
//      case Column::COMMAND:
//        return QString::fromStdString(button.action.get());
//      case Column::BUTTON:
//      case Column::NAME:
//        return QString::fromStdString(button.name.get());
//      case Column::ICON:
//        return QString::fromStdString(button.file.get());
//      case Column::POSITION: {
//        auto pos = button.position.get();
//        return tr("%1 (Page %2, Button %3)").arg(pos).arg(
//            pos / SOFTBUTTONS_PER_PAGE).arg(pos % SOFTBUTTONS_PER_PAGE);
//      }
//      default:
//        break;
//    }
//  } catch (const out_of_range &ex) {
//  }
//  return {};
//}
//
//QVariant ButtonBaseModel::getEditData(const QModelIndex &index) const
//{
//  try {
//    auto &button = getButtons().at(index.row());
//    switch (index.column()) {
//      case Column::DEVICE:
//        return button.device.get();
//      case Column::COMMAND:
//        return QString::fromStdString(button.action.get());
//      case Column::BUTTON:
//      case Column::NAME:
//        return QString::fromStdString(button.name.get());
//      case Column::ICON:
//        return QString::fromStdString(button.file.get());
//      case Column::POSITION:
//        return getDisplayData(index); //todo that is not editdata...
//      default:
//        break;
//    }
//  } catch (const out_of_range &ex) {
//  }
//  return {};
//}
//
//QVariant ButtonBaseModel::getTooltipData(const QModelIndex &index) const
//{
//  try {
//    return columnSetup.at(static_cast<Column>(index.column())).context;
//  } catch (...) {
//  }
//  return {};
//}
//
//QVariant ButtonBaseModel::getBackgroundData(const QModelIndex &index) const
//{
//  return {};
//}
//
//QVariant ButtonBaseModel::getForegroundData(const QModelIndex &index) const
//{
//  return {};
//}
//
//QVariant ButtonBaseModel::getSelectionItemsData(const QModelIndex &index) const
//{
//  try {
//    auto &button = getButtons().at(index.row());
//    switch (index.column()) {
//      case Column::DEVICE: {
//        auto list = config.data().getDeviceLabels();
//        return lib::toQStringList(list);
//      }
//      case Column::COMMAND:
//
//        //todo
//
//        break;
//      case Column::BUTTON:
//        return getSelectionItemsDataButton(button);
//      case Column::ICON:
//        return columnSetup   ---- icons;
//      case Column::POSITION:
//        return getSelectionItemsDataPosition(button);
//      default:
//        break;
//    }
//  } catch (const out_of_range &ex) {
//  }
//  return {};
//}
//
//QVariant ButtonBaseModel::getSelectionItemsDataButton(
//    const document::data::item::Button &button) const
//{
//  //only allow unused buttons
//  auto current = QString::fromStdString(button.name.get());
//  auto list =
//      document::data::Enum<document::data::HardButtons>(current).getQStringList();
//  for (const auto button : getButtons()) {
//    auto name = QString::fromStdString(button.name.get());
//    if (name == current) {
//      continue;
//    }
//    list.removeAll(name);
//  }
//  return list;
//}
//
//QVariant ButtonBaseModel::getSelectionItemsDataPosition(
//    const document::data::item::Button &button) const
//{
////only allow each position once. always add a new, empty
////page once a page has at least one button on it.
//  int i;
//  set<int> list;
//  QStringList positions;
//  int buttonCount = SOFTBUTTONS_PER_PAGE; //min one page
//
//  auto current = button.position.get();
//
////get all positions
//  for (const auto button : getButtons()) {
//    auto pos = button.position.get();
//    if (pos >= 0) {
//      list.insert(pos);
//    }
//  }
////find the button with the highest index
//  if (!list.empty()) {
//    auto last = *list.rbegin();
//    auto pages = (last + SOFTBUTTONS_PER_PAGE - 1) % SOFTBUTTONS_PER_PAGE;
//    buttonCount = (pages + 1) * SOFTBUTTONS_PER_PAGE; //add one empty page
//  }
////create entries for all others (and current entry)
//  for (i = 0; i < buttonCount; i++) {
//    if (list.contains(i) && (i != current)) {
//      continue;
//    }
//    positions.push_back(
//        tr("%1 (Page %2, Button %3)").arg(i).arg(i / SOFTBUTTONS_PER_PAGE).arg(
//            i % SOFTBUTTONS_PER_PAGE));
//  }
//  return positions;
//}

}

