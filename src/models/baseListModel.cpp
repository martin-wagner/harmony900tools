// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "baseListModel.h"

using namespace std;

namespace models
{

BaseModel::BaseModel(document::data::Item item, QObject *parent) :
    QAbstractItemModel(parent), item(item)
{
}

BaseModel::~BaseModel() = default;

QVariant BaseModel::data(const QModelIndex &index, int role) const
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
    case Qt::CheckStateRole:
      return getCheckStateData(index);
    case Qt::ToolTipRole:
      return getTooltipData(index);
    case Qt::BackgroundRole:
      return getBackgroundData(index);
    case Qt::ForegroundRole:
      return getForegroundData(index);
    case Qt::FontRole:
      return getFontData(index);
    case UserDataRole::SelectionItemsRole:
      return getSelectionItemsData(index);
    default:
      return {};
  }
}

QModelIndex BaseModel::parent(const QModelIndex &index) const
{
  return {};
}

bool BaseModel::setHeaderData(int section, Qt::Orientation orientation,
    const QVariant &value, int role)
{
  return false;
}

bool BaseModel::insertColumns(int position, int columns,
    const QModelIndex &parent)
{
  return false;
}

bool BaseModel::removeColumns(int position, int columns,
    const QModelIndex &parent)
{
  return false;
}

void models::BaseModel::createActions(document::Config *config)
{
  connect(config, &document::Config::itemChanged, this,
      &BaseModel::itemChangedObserver);
  connect(config, &document::Config::itemAboutToBeAdded, this,
      &BaseModel::itemAboutToBeAddedObserver);
  connect(config, &document::Config::itemAdded, this,
      &BaseModel::itemAddedObserver);
  connect(config, &document::Config::itemAboutToBeRemoved, this,
      &BaseModel::itemAboutToBeRemovedObserver);
  connect(config, &document::Config::itemRemoved, this,
      &BaseModel::itemRemovedObserver);
}

QVariant BaseModel::getDisplayData(const QModelIndex &index) const
{
  return {};
}

QVariant BaseModel::getEditData(const QModelIndex &index) const
{
  return {};
}

QVariant BaseModel::getCheckStateData(const QModelIndex &index) const
{
  return {};
}

QVariant BaseModel::getTooltipData(const QModelIndex &index) const
{
  return {};
}

QVariant BaseModel::getBackgroundData(const QModelIndex &index) const
{
  return {};
}

QVariant BaseModel::getForegroundData(const QModelIndex &index) const
{
  return {};
}

QVariant BaseModel::getFontData(const QModelIndex &index) const
{
  return {};
}

QVariant BaseModel::getSelectionItemsData(const QModelIndex &index) const
{
  return {};
}

QString BaseModel::makeStringUnique(const QStringList &input, QString str)
{
  auto baseStr = str;

  int suffix = 1;
  while (input.contains(str)) {
    str = baseStr + QString::number(suffix);
    suffix++;
  }
  if (suffix > 1) {
    emit writeMsg(tr("%1 already used. Names must be unique").arg(baseStr));
  }

  return str;
}

QStringList BaseModel::toQStringList(const vector<string> &list)
{
  QStringList qlist;
  qlist.reserve(static_cast<qsizetype>(list.size()));

  for (const auto &i : list) {
    qlist.push_back(QString::fromStdString(i));
  }
  return qlist;
}

vector<string> BaseModel::toStringList(const QStringList &qlist)
{
  vector<string> list;
  list.reserve(static_cast<size_t>(qlist.size()));

  for (const auto &i : qlist) {
    list.push_back(i.toStdString());
  }
  return list;
}

void BaseModel::itemChangedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  //we don't know the column!
  emit dataChanged(index(pos, 0), index(pos, columnCount()), {
    Qt::DisplayRole,
    Qt::EditRole });
}

void BaseModel::itemAboutToBeAddedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit beginInsertRows(QModelIndex(), pos, pos);
}

void BaseModel::itemAddedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit endInsertRows();
}

void BaseModel::itemAboutToBeRemovedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit beginRemoveRows(QModelIndex(), pos, pos);
}

void BaseModel::itemRemovedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit endRemoveRows();
}

}

