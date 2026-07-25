// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "document/data/catalogue.h"
#include "base.h"

namespace document
{
  class Config;
  namespace data {
    class ConfigData;
    namespace item {
      class Activity;
    }
  }
}

namespace models
{

//Model for Activitys
class ActivityModel: public QAbstractItemModel
{
  Q_OBJECT
  public:
    enum Column {
      ID,
      ACTTYPE,
      LABEL,

      COUNT
    };

    const std::map<Column, Setup> columnSetup = {
        { Column::ID,            { "ID", "id", "int", true,  {}, } },
        { Column::ACTTYPE,       { "Type", "What kind of Activity you have", "Enum", false, {}, } },
        { Column::LABEL,         { "Name", "Activity Name (displayed on \"My Activities\" screen)", "QString", false, {}, } },
    };

  public:
    ActivityModel(document::Config &config, QObject *parent = nullptr);
    ~ActivityModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = { }) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = { }) const override;
    int columnCount(const QModelIndex &parent = { }) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns, const QModelIndex &parent = { }) override;
    bool removeColumns(int position, int columns, const QModelIndex &parent = { }) override;
    bool insertRows(int position, int rows, const QModelIndex &parent = { }) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = { }) override;

  private:
    document::Config &config;
    QStringList header;

    void createActions();

    QVariant getDisplayData(const QModelIndex &index) const;
    QVariant getEditData(const QModelIndex &index) const;
    QVariant getTooltipData(const QModelIndex &index) const;
    QVariant getBackgroundData(const QModelIndex &index) const;
    QVariant getForegroundData(const QModelIndex &index) const;
    QVariant getSelectionItemsData(const QModelIndex &index) const;

  private slots:
    void itemChangedObserver(document::data::Item item, int pos);
    void itemAboutToBeAddedObserver(document::data::Item item, int pos);
    void itemAddedObserver(document::data::Item item, int pos);
    void itemAboutToBeRemovedObserver(document::data::Item item, int pos);
    void itemRemovedObserver(document::data::Item item, int pos);
};

}

