// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "deviceListModel.h"
#include "lib/qtHelpers.h"

using namespace std;

namespace models
{

DeviceModel::DeviceModel(document::Config &config, QObject *parent) :
    BaseModel(document::data::Item::DEVICE, parent), config(config)
{
  createActions(&config);
}

DeviceModel::~DeviceModel() = default;

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
  if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
    return {};
  }
  try {
    return columnSetup.at(static_cast<Column>(section)).name;
  } catch (...) {
  }
  return {};
}

QModelIndex DeviceModel::index(int row, int column,
    const QModelIndex &parent) const
{
  if (parent.isValid()) {
    //not a hierarchical model! parent = header, data = first.
    return {};
  }
  if ((row >= config.data().getDevices().size()) || (column >= Column::COUNT)) {
    return {};
  }
  return createIndex(row, column, &config.data().getDevices()[row]);
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return config.data().getDevices().size();
}

int DeviceModel::columnCount(const QModelIndex &parent) const
{
  return columnSetup.size();
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  try {
    auto isConst = columnSetup.at(static_cast<Column>(index.column())).isConst;
    if (!isConst) {
      return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
  } catch (...) {
  }
  return QAbstractItemModel::flags(index);
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value,
    int role)
{
  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto column = static_cast<Column>(index.column());

  //checks
  if (!columnSetup.contains(column)) {
    return false;
  }
  if (columnSetup.at(column).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  auto &worker = config.modify();
  switch (index.column()) {
    case Column::DEVTYPE:
      return worker.setDeviceType(
          document::data::Enum<document::data::DeviceType>(value.toString()),
          row);
    case Column::MANUFACTURER:
      return worker.setDeviceMnf(value.toString(), row);
    case Column::MODEL:
      return worker.setDeviceModel(value.toString(), row);
    case Column::NAME:
      return setDeviceName(worker, row, value);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

bool DeviceModel::insertRows(int position, int rows, const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }
  auto &worker = config.modify();

  //no begin/end insert rows, is done inside observers

  config.beginMacro(QObject::tr("Add %1 device(s)").arg(rows));

  for (int i = position; i < position + rows; i++) {
    auto ret = worker.addDeviceCommand(i);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool DeviceModel::removeRows(int position, int rows, const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }
  auto &devices = config.data().getDevices();
  if ((position + rows) > devices.size()) {
    return false;
  }
  auto &worker = config.modify();

  //no begin/end remove rows, is done inside observers

  config.beginMacro(QObject::tr("Remove %1 device(s)").arg(rows));

  for (int i = 0; i < rows; i++) {
    //remove beginning at last item
    auto currRow = position + rows - 1 - i;
    auto ret = worker.removeDeviceCommand(currRow);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

QVariant DeviceModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto &device = config.data().getDevices().at(index.row());
    switch (index.column()) {
      case Column::ID:
        return device.getId();
      case Column::DEVTYPE:
        return device.type.get().getQString();
      case Column::MANUFACTURER:
        return QString::fromStdString(device.mnf.get());
      case Column::MODEL:
        return QString::fromStdString(device.model.get());
      case Column::NAME:
        return QString::fromStdString(device.label.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &device = config.data().getDevices().at(index.row());
    switch (index.column()) {
      case Column::ID:
        return device.getId();
      case Column::DEVTYPE:
        return device.type.get().getQString();
      case Column::MANUFACTURER:
        return QString::fromStdString(device.mnf.get());
      case Column::MODEL:
        return QString::fromStdString(device.model.get());
      case Column::NAME:
        return QString::fromStdString(device.label.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(static_cast<Column>(index.column())).context;
  } catch (...) {
  }
  return {};
}

QVariant DeviceModel::getSelectionItemsData(const QModelIndex &index) const
{
  try {
    auto &device = config.data().getDevices().at(index.row());
    switch (index.column()) {
      case Column::DEVTYPE:
        return device.type.get().getQStringList();
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

bool DeviceModel::setDeviceName(document::data::CmdCatalogue &worker, int row,
    const QVariant &value)
{
  QStringList usedNames;

  auto &devices = config.data().getDevices();
  for (const auto &activity : devices) {
    usedNames.push_back(QString::fromStdString(activity.label.get()));
  }

  auto name = makeStringUnique(usedNames, value.toString());
  return worker.setDeviceLabel(name, row);
}

}

