// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "rawIrListModel.h"
#include "lib/qtHelpers.h"

using namespace std;

namespace models
{

RawIrModel::RawIrModel(document::Config &config, uint32_t deviceId,
    QObject *parent) :
    BaseModel(document::data::Item::DEVICE_IR_DATA, parent), config(config), id(
        deviceId)
{
  createActions(&config);
}

RawIrModel::~RawIrModel() = default;

QVariant RawIrModel::headerData(int section, Qt::Orientation orientation,
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

QModelIndex RawIrModel::index(int row, int column,
    const QModelIndex &parent) const
{
  if (parent.isValid()) {
    //not a hierarchical model! parent = header, data = first.
    return {};
  }
  if ((row >= getCmds().size()) || (column >= Column::COUNT)) {
    return {};
  }
  return createIndex(row, column, &getCmds()[row]);
}

int RawIrModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return getCmds().size();
}

int RawIrModel::columnCount(const QModelIndex &parent) const
{
  return columnSetup.size();
}

Qt::ItemFlags RawIrModel::flags(const QModelIndex &index) const
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

bool RawIrModel::setData(const QModelIndex &index, const QVariant &value,
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
    switch (index.column()) {
      case Column::NAME:
        return setCommandName(worker, row, value);
      case Column::DATACLOCK: {
        auto cmd = getCmds(&devicePos).at(row);
        cmd.stream.setClock(value.toDouble());
        return worker.setIrCommand(devicePos, cmd, row, true);
      }
      case Column::DATA: {
        //also overwrites clock!
        auto cmd = getCmds(&devicePos).at(row);
        auto stream = qvariant_cast<binary::ssIr::SerialStreamIr>(value);
        cmd.stream = stream;
        return worker.setIrCommand(devicePos, cmd, row, true);
      }
      default:
        return false;
    }
  } catch (const out_of_range &ex) {
    return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

bool RawIrModel::insertRows(int position, int rows, const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }

  //no begin/end insert rows, is done inside observers

  config.beginMacro(QObject::tr("Add %1 raw ir cmd(s)").arg(rows));

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

bool RawIrModel::removeRows(int position, int rows, const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }
  auto &rawIrs = config.data().getDevices();
  if ((position + rows) > rawIrs.size()) {
    return false;
  }

  //no begin/end remove rows, is done inside observers

  config.beginMacro(QObject::tr("Remove %1 raw ir cmd(s)").arg(rows));

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

bool models::RawIrModel::addItem(int row)
{
  uint32_t devicePos;
  QString commandToUse;
  document::data::item::RawCommand cmd;

  QString suggestName = tr("New Command");

  auto *device = config.data().getDevice(id, &devicePos);

  //auto assign names. try all hard buttons, then default to "new command"
  auto names =
      document::data::Enum<document::data::HardButtons>::toQStringList(); //this is logically sorted
  auto usedNames = toQStringList(
      device->getIrCommands().getAvailableCommands());
  for (const auto &name : names) {
    if (!usedNames.contains(name)) {
      suggestName = name;
    }
  }
  suggestName = makeStringUnique(usedNames, suggestName);
  cmd.name = suggestName.toStdString();
  cmd.stream.addData( { 300, 300, 300 }); //add some dummy data, clock is pre-set
  return config.modify().setIrCommand(devicePos, cmd, row, false);
}

bool models::RawIrModel::removeItem(int row)
{
  uint32_t devicePos;

  config.data().getDevice(id, &devicePos);

  return config.modify().removeIrRawCommand(devicePos, row);
}

const std::vector<document::data::item::RawCommand>& RawIrModel::getCmds(
    uint32_t *devicePos) const
{
  return config.data().getDevice(id, devicePos)->getIrCommands().getRawCommands();
}

QVariant RawIrModel::getDisplayData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &cmd = getCmds(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::NAME:
        return QString::fromStdString(cmd.name.get());
      case Column::DATACLOCK:
        return cmd.stream.getClock();
      case Column::DATA: {
        auto str =
            cmd.stream.accessStream().convertAsciiPlot(250, true, false);
        while (!str.empty() && str.back() == '\n') {
          str.pop_back();
        }
        return QString::fromStdString(str); }
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant RawIrModel::getEditData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &cmd = getCmds(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::DATACLOCK:
        return cmd.stream.getClock();
      case Column::DATA: {
        return QVariant::fromValue(cmd.stream);
      }
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant RawIrModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(static_cast<Column>(index.column())).context;
  } catch (...) {
  }
  return {};
}

bool RawIrModel::setCommandName(document::data::CmdCatalogue &worker, int row,
    const QVariant &value)
{
  uint32_t devicePos;
  QStringList usedNames;

  auto &cmds = getCmds(&devicePos);
  for (const auto &cmd : cmds) {
    usedNames.push_back(QString::fromStdString(cmd.name.get()));
  }

  auto name = makeStringUnique(usedNames, value.toString());
  auto cmd = cmds.at(row);
  cmd.name = name.toStdString();
  return worker.setIrCommand(devicePos, cmd, row, true);
}

}

