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

//Model for raw commands. this model uses data/device/command/rawcommand + device/streamlib
class RawIrModel: public BaseModel
{
  Q_OBJECT
  public:
    enum Column {
      NAME,
      DATACLOCK,
      DATA,

      COUNT
    };

    const std::map<Column, Setup> columnSetup = {
        { Column::NAME,          { "Name", "The command name is used to reference this command in other tables (Buttons, ...).", "QString", false, {}, } },
        { Column::DATACLOCK,     { "Data clock / Hz", "IR Command data clock. Auto-detected at learn time, don't change unless you know what you are doing.", "int", false, {}, } },
        { Column::DATA,          { "Data", "- Select cell to enable learning. Needs remote to be connected.\n- Double-click to edit manually\n\nVisual representation of IR data. If you only see a flat line, learning most likely has failed.", "SerialStreamIr", false, {}, } }
    };

  public:
    RawIrModel(document::Config &config, uint32_t deviceId, QObject *parent = nullptr);
    ~RawIrModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = { }) const override;

    int rowCount(const QModelIndex &parent = { }) const override;
    int columnCount(const QModelIndex &parent = { }) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int position, int rows, const QModelIndex &parent = { }) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = { }) override;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    bool addItem(int row);
    bool removeItem(int row);

  private:
    document::Config &config;
    uint32_t id;

    const std::vector<document::data::item::RawCommand> &getCmds(uint32_t *devicePos = nullptr) const;
    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getFontData(const QModelIndex &index) const override;

    bool setCommandName(document::data::CmdCatalogue &worker, int row, const QVariant &value);
};

}

