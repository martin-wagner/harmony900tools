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

  auto flags = QAbstractItemModel::flags(index);
  try {
    auto isConst = columnSetup.at(static_cast<Column>(index.column())).isConst;
    if (!isConst) {
      flags = flags | Qt::ItemIsEditable;
    }
    auto dataType = columnSetup.at(static_cast<Column>(index.column())).dataType;
    if (dataType == "bool") {
      flags = flags | Qt::ItemIsUserCheckable;
    }
  } catch (...) {
  }
  return flags;
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value,
    int role)
{
  if (index.parent().isValid()
      || ((role != Qt::EditRole) && (role != Qt::CheckStateRole))) {
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

  switch (role) {
    case Qt::EditRole:
      return setDataValue(index, value);
    case Qt::CheckStateRole:
      return setDataCheck(index, value);
    default:
      return false;
  }
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
      case Column::DISPLAY:
        return device.isDisplayDevice.get();
      case Column::ALWAYS_ON:
        return device.alwaysOn.get();
      case Column::MANUAL_POWER:
        return device.manualPower.get();
      case Column::SCART:
        return device.scart.get();
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceModel::getCheckStateData(const QModelIndex &index) const
{
  bool value = false;

  try {
    auto &device = config.data().getDevices().at(index.row());
    switch (index.column()) {
      case Column::DISPLAY:
        value = device.isDisplayDevice.get();
        break;
      case Column::ALWAYS_ON:
        value = device.alwaysOn.get();
        break;
      case Column::MANUAL_POWER:
        value = device.manualPower.get();
        break;
      case Column::SCART:
        value = device.scart.get();
        break;
      default:
        return {};
    }
  } catch (const out_of_range &ex) {
    return {};
  }
  if (value == true) {
    return Qt::Checked;
  }
  return Qt::Unchecked;
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

bool DeviceModel::setDataValue(const QModelIndex &index, const QVariant &value)
{
  auto row = index.row();

  auto &worker = config.modify();
  switch (index.column()) {
    case Column::DEVTYPE: {
      config.beginMacro("Set device type");
      auto deviceType = document::data::Enum<document::data::DeviceType>(
          value.toString());
      switch (deviceType.getValue()) {
        case document::data::DeviceType::Projector:
        case document::data::DeviceType::Television:
          worker.setDeviceIsDisplayDevice(true, row);
          break;
        default:
          worker.setDeviceIsDisplayDevice(false, row);
          break;
      }
      auto ret = worker.setDeviceType(deviceType, row);
      config.endMacro();
      return ret;
    }
    case Column::MANUFACTURER:
      return worker.setDeviceMnf(value.toString(), row);
    case Column::MODEL:
      return worker.setDeviceModel(value.toString(), row);
    case Column::NAME:
      return setDeviceName(worker, row, value);
    case Column::DISPLAY:
      return worker.setDeviceIsDisplayDevice(value.toBool(), row);
    case Column::ALWAYS_ON:
      return worker.setDeviceAlwaysOn(value.toBool(), row);
    case Column::MANUAL_POWER:
      return worker.setDeviceManualPower(value.toBool(), row);
    case Column::SCART:
      return worker.setDeviceScart(value.toBool(), row);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

bool DeviceModel::setDataCheck(const QModelIndex &index, const QVariant &value)
{
  auto row = index.row();
  bool checkedState;

  checkedState = false;
  if (value.toInt() == Qt::Checked) {
    checkedState = true;
  }

  auto &worker = config.modify();
  switch (index.column()) {
    case Column::DISPLAY:
      return worker.setDeviceIsDisplayDevice(checkedState, row);
    case Column::ALWAYS_ON:
      return worker.setDeviceAlwaysOn(checkedState, row);
    case Column::MANUAL_POWER:
      return worker.setDeviceManualPower(checkedState, row);
    case Column::SCART:
      return worker.setDeviceScart(checkedState, row);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
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

