// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "activityListModel.h"

using namespace std;

namespace models
{

ActivityModel::ActivityModel(document::Config &config, QObject *parent) :
    BaseModel(document::data::Item::ACTIVITY, parent), config(config)
{
  createActions(&config);
}

ActivityModel::~ActivityModel() = default;

QVariant ActivityModel::headerData(int section, Qt::Orientation orientation,
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

QModelIndex ActivityModel::index(int row, int column,
    const QModelIndex &parent) const
{
  if (parent.isValid()) {
    //not a hierarchical model! parent = header, data = first.
    return {};
  }
  if ((row >= config.data().getActivities().size())
      || (column >= Column::COUNT)) {
    return {};
  }
  return createIndex(row, column, &config.data().getActivities()[row]);
}

int ActivityModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return config.data().getActivities().size();
}

int ActivityModel::columnCount(const QModelIndex &parent) const
{
  return columnSetup.size();
}

Qt::ItemFlags ActivityModel::flags(const QModelIndex &index) const
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

bool ActivityModel::setData(const QModelIndex &index, const QVariant &value,
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
    case Column::ACTTYPE:
      return worker.setActivityType(
          document::data::Enum<document::data::ActivityType>(value.toString()),
          row);
    case Column::LABEL:
      return setActivityName(worker, row, value);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

bool ActivityModel::insertRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }
  auto &worker = config.modify();

  //no begin/end insert rows, is done inside observers

  config.beginMacro(QObject::tr("Add %1 Activitie(s)").arg(rows));

  for (int i = position; i < position + rows; i++) {
    auto ret = worker.addActivityCommand(i);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool ActivityModel::removeRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }
  auto &activities = config.data().getActivities();
  if ((position + rows) > activities.size()) {
    return false;
  }
  auto &worker = config.modify();

  //no begin/end remove rows, is done inside observers

  config.beginMacro(QObject::tr("Remove %1 Activitie(s)").arg(rows));

  for (int i = 0; i < rows; i++) {
    //remove beginning at last item
    auto currRow = position + rows - 1 - i;
    auto ret = worker.removeActivityCommand(currRow);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool ActivityModel::moveRows(const QModelIndex &sourceParent, int sourceRow,
    int count, const QModelIndex &destinationParent, int destinationChild)
{
  if (sourceParent.isValid() || destinationParent.isValid()) {
    return false;
  }
  if (count != 1) {
    //not implemented
    return false;
  }
  auto &activities = config.data().getActivities();
  if ((sourceRow > activities.size()) || (destinationChild > activities.size())) {
    return false;
  }
  auto &worker = config.modify();
  return worker.moveActivityCommand(sourceRow, destinationChild);
}

QVariant ActivityModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto &activity = config.data().getActivities().at(index.row());
    switch (index.column()) {
      case Column::ID:
        return activity.getId();
      case Column::ACTTYPE:
        return activity.type.get().getQString();
      case Column::LABEL:
        return QString::fromStdString(activity.label.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ActivityModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &activity = config.data().getActivities().at(index.row());
    switch (index.column()) {
      case Column::ID:
        return activity.getId();
      case Column::ACTTYPE:
        return activity.type.get().getQString();
      case Column::LABEL:
        return QString::fromStdString(activity.label.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ActivityModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(static_cast<Column>(index.column())).context;
  } catch (...) {
  }
  return {};
}

QVariant ActivityModel::getSelectionItemsData(const QModelIndex &index) const
{
  try {
    auto &activity = config.data().getActivities().at(index.row());
    switch (index.column()) {
      case Column::ACTTYPE:
        return activity.type.get().getQStringList();
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

bool ActivityModel::setActivityName(document::data::CmdCatalogue &worker, int row,
    const QVariant &value)
{
  QStringList usedNames;

  auto &activities = config.data().getActivities();
  for (const auto &activity : activities) {
    usedNames.push_back(QString::fromStdString(activity.label.get()));
  }

  auto name = makeStringUnique(usedNames, value.toString());
  return worker.setActivityLabel(name, row);
}

}

