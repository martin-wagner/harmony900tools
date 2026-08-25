// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "statemachineListModel.h"
#include "lib/qtHelpers.h"

using namespace std;

namespace models
{

StateMachineModel::StateMachineModel(document::Config &config,
    uint32_t deviceId, QObject *parent) :
    BaseModel(document::data::Item::DEVICE_STATEMACHINE, parent), config(config), id(
        deviceId)
{
  createActions(&config);
}

StateMachineModel::~StateMachineModel() = default;

QVariant StateMachineModel::headerData(int section, Qt::Orientation orientation,
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

QModelIndex StateMachineModel::index(int row, int column,
    const QModelIndex &parent) const
{
  if (parent.isValid()) {
    //not a hierarchical model! parent = header, data = first.
    return {};
  }
  if ((row >= getMachines().size()) || (column >= Column::COUNT)) {
    return {};
  }
  return createIndex(row, column, &getMachines()[row]);
}

int StateMachineModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return getMachines().size();
}

int StateMachineModel::columnCount(const QModelIndex &parent) const
{
  return columnSetup.size();
}

Qt::ItemFlags StateMachineModel::flags(const QModelIndex &index) const
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

bool StateMachineModel::setData(const QModelIndex &index, const QVariant &value,
    int role)
{
  uint32_t devicePos;

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
  try {
    config.data().getDevice(id, &devicePos);
    switch (index.column()) {
      case Column::CONTROL_TYPE:
        return worker.setDeviceStatemachineType(
            document::data::Enum<document::data::StateMachineDeviceType>(
                value.toString()), devicePos, row);
      default:
        return false;
    }
  } catch (const out_of_range &ex) {
    return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

bool StateMachineModel::insertRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }

  //no begin/end insert rows, is done inside observers

  config.beginMacro(QObject::tr("Add %1 state machine(s)").arg(rows));

  for (int i = position; i < position + rows; i++) {
    auto ret = addItem(i);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool StateMachineModel::removeRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }

  //no begin/end remove rows, is done inside observers

  config.beginMacro(QObject::tr("Remove %1 state machine(s)").arg(rows));

  for (int i = 0; i < rows; i++) {
    //remove beginning at last item
    auto currRow = position + rows - 1 - i;
    auto ret = removeItem(currRow);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool models::StateMachineModel::addItem(int row)
{
  uint32_t devicePos;
  QStringList usedCtrlTypes;
  QString suggestCtrlType;
  QString commandToUse;
  document::data::item::RawCommand cmd;

  auto &m = getMachines(&devicePos);

  //find first unused control type, use that one.
  auto ctrlTypes =
      document::data::Enum<document::data::StateMachineDeviceType>::toQStringList(); //this is logically sorted
  for (const auto &machine : m) {
    usedCtrlTypes.push_back(machine.smType.get().getQString());
  }
  for (const auto &ctrlType : ctrlTypes) {
    if (!usedCtrlTypes.contains(ctrlType)) {
      suggestCtrlType = ctrlType;
      break;
    }
  }
  if (suggestCtrlType.isEmpty()) {
    emit writeMsg(tr("All state machine types are used!"));
    return false;
  }

  auto ret = config.modify().addDeviceStatemachineCommand(devicePos, row);
  if (ret != true) {
    return ret;
  }
  return config.modify().setDeviceStatemachineType(
      document::data::Enum<document::data::StateMachineDeviceType>(
          suggestCtrlType), devicePos, row);
}

bool models::StateMachineModel::removeItem(int row)
{
  uint32_t devicePos;

  config.data().getDevice(id, &devicePos);

  return config.modify().removeDeviceStatemachineCommand(devicePos, row);
}

const std::vector<document::data::item::StateMachine>& StateMachineModel::getMachines(
    uint32_t *devicePos) const
{
  return config.data().getDevice(id, devicePos)->getStateMachines();
}

QVariant StateMachineModel::getDisplayData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &m = getMachines(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::CONTROL_TYPE:
        return m.smType.get().getQString();
      case Column::MACHINE_TYPE:
        if (!m.discrete.empty()) {
          return tr("Direct select");
        } else if (!m.relative.empty()) {
          return tr("Cycle");
        } else {
          return tr("Start Wizard");
        }
        break;
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant StateMachineModel::getEditData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &m = getMachines(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::CONTROL_TYPE:
        return m.smType.get().getQString();
      case Column::MACHINE_TYPE:
        if (!m.discrete.empty()) {
          return static_cast<int>(document::data::item::StateMachineType::Discrete);
        } else if (!m.relative.empty()) {
          return static_cast<int>(document::data::item::StateMachineType::Relative);
        } else {
          return static_cast<int>(document::data::item::StateMachineType::Unknown);
        }
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant StateMachineModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(static_cast<Column>(index.column())).context;
  } catch (...) {
  }
  return {};
}

QVariant StateMachineModel::getSelectionItemsData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &m = getMachines(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::CONTROL_TYPE:
        return m.smType.get().getQStringList();
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

}

