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

//Model for protocol commands. this model uses data/device/command/protocommand
class ProtocolIrModel: public BaseModel
{
  Q_OBJECT
  public:
    enum Column {
      NAME,
      TYPE,
      DATA,

      COUNT
    };

    const std::map<Column, Setup> columnSetup = {
        { Column::NAME,          { "Name", "The command name is used to reference this command in other tables (Buttons, ...).", "QString", false, {}, } },
        { Column::TYPE,          { "Protocol", "IR Protocol name. \"Proprietary\" means from your original data set.", "Enum", false, {} } },
        { Column::DATA,          { "Data", "- Select cell to enable learning. Needs remote to be connected.\n- Double-click to edit manually\n\nVisual representation of IR data..", "Code", false, {}, } }
    };

  public:
    ProtocolIrModel(document::Config &config, uint32_t deviceId, QObject *parent = nullptr);
    ~ProtocolIrModel() override;

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

    const std::vector<document::data::item::ProtocolCommand> &getCmds(uint32_t *devicePos = nullptr) const;
    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getFontData(const QModelIndex &index) const override;

    bool setCommandName(document::data::CmdCatalogue &worker, int row, const QVariant &value);
};

}

