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

//Model for devices
class DeviceModel: public BaseModel
{
  Q_OBJECT
  public:
    enum Column {
      ID,
      DEVTYPE,
      NAME,
      MANUFACTURER,
      MODEL,

      COUNT
    };

    const std::map<Column, Setup> columnSetup = {
        { Column::ID,            { "ID", "id", "int", true,  {}, } },
        { Column::DEVTYPE,       { "Type", "What kind of device you have", "Enum", false, {}, } },
        { Column::NAME,          { "Name", "Device name (on \"Devices\" screen)", "QString", false, {}, } },
        { Column::MANUFACTURER,  { "Manufacturer", "Device manufacturer", "QString", false, {}, } },
        { Column::MODEL,         { "Model", "Device model", "QString", false, {}, } }
    };

  public:
    DeviceModel(document::Config &config, QObject *parent = nullptr);
    ~DeviceModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = { }) const override;

    int rowCount(const QModelIndex &parent = { }) const override;
    int columnCount(const QModelIndex &parent = { }) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int position, int rows, const QModelIndex &parent = { }) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = { }) override;

  private:
    document::Config &config;

    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getSelectionItemsData(const QModelIndex &index) const override;

    bool setDeviceName(document::data::CmdCatalogue &worker, int row, const QVariant &value);

};

}

