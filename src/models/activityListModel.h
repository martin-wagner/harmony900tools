// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "document/data/catalogue.h"
#include "baseListModel.h"

namespace document
{
  class Config;
}

namespace models
{

//Model for Activitys
class ActivityModel: public BaseModel
{
  Q_OBJECT
  public:
    enum Column {
      ID,
      ACTTYPE,
      LABEL,
      POWER_OFF,
      AUTO_PLAY,
      AUTO_STOP,
      TRAINING,

      COUNT
    };

    const std::map<Column, Setup> columnSetup = {
        { Column::ID,            { "ID", "id", "int", true,  {}, } },
        { Column::ACTTYPE,       { "Type", "What kind of Activity you have", "Enum", false, {}, } },
        { Column::LABEL,         { "Name", "Activity Name (displayed on \"My Activities\" screen)", "QString", false, {}, } },
        { Column::POWER_OFF,     { "Auto off", "Power off unused devices when starting this activity", "bool", false, {}, } },
        { Column::AUTO_PLAY,     { "Auto play", "Start playing content when starting this activity", "bool", false, {}, } },
        { Column::AUTO_STOP,     { "Auto stop", "Stop playing content when leaving this activity", "bool", false, {}, } },
        { Column::TRAINING,      { "Training", "Start \"testing mode\" on the remote for this activity. Useful when you add/change a activity", "bool", false, {}, } },
    };

  public:
    ActivityModel(document::Config &config, QObject *parent = nullptr);
    ~ActivityModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = { }) const override;

    int rowCount(const QModelIndex &parent = { }) const override;
    int columnCount(const QModelIndex &parent = { }) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int position, int rows, const QModelIndex &parent = { }) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = { }) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);

  private:
    document::Config &config;

    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getCheckStateData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getSelectionItemsData(const QModelIndex &index) const override;

    bool setDataValue(const QModelIndex &index, const QVariant &value);
    bool setDataCheck(const QModelIndex &index, const QVariant &value);

    bool setActivityName(document::data::CmdCatalogue &worker, int row, const QVariant &value);
};

}

