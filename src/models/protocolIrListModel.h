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
      VERBOSE,
      IRADDRESS,
      IRCOMMAND,
      IRBITS,
      PROTO,
      DATA,

      COUNT
    };

    const std::map<Column, Setup> columnSetup = {
        { Column::NAME,          { "Name", "The command name is used to reference this command in other tables (Buttons, ...).", "QString", false, {}, } },
        { Column::TYPE,          { "Protocol", "IR Protocol name. \"Proprietary\" means created by the Harmony 900 software.\n"
            "- NEC: Most common, used by NEC, Yamaha, Canon, Tevion, Harman/Kardon, Hitachi, JVC, Pioneer, Toshiba, Xoro, Orion, Apple, many NoNames, etc.\n"
            "- KASEIKYO: Mostly japanese, used by Panasonic, Denon, Technics, and other AEHA manufacturers.\n"
            "- Philips RC5: Mostly european manufacturers, including Philips, older / simpler devices.\n"
            "- Philips RC6: Philips.\n"
            "- Philips RC6A: Mostly european manufacturers, newer devices.\n"
            "- SIRCS: Sony.\n"
            "- Samsung32: Samsung.", "Enum", false, {} } },
        { Column::VERBOSE,       { "Detail", "Verbose name of protocol type (from IRremoteESP8266 lib)", "int", false, {}, } },
        { Column::IRADDRESS,     { "IR Address", "IR Device Address", "int", false, {}, } },
        { Column::IRCOMMAND,     { "IR Command", "IR Device Command (=Button)", "int", false, {}, } },
        { Column::IRBITS,        { "IR Bits", "IR raw command bits", "int", false, {}, } },
        { Column::PROTO,         { "Protocol Index", "Protocol index", "int", true, {}, } },
        { Column::DATA,          { "Data", "- Select cell to enable learning. Needs remote to be connected.\n- Double-click to edit manually\n\nVisual representation of IR data..", "Code", false, {}, } },
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

    const std::vector<document::data::item::ProtoCommand> &getCmds(uint32_t *devicePos = nullptr) const;
    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getSelectionItemsData(const QModelIndex &index) const override;
    QVariant getFontData(const QModelIndex &index) const override;

    bool printData(const document::data::item::ProtoCommand &cmd) const;
    QVariant visualiseData(const document::data::item::ProtoCommand &cmd) const;

    bool setCommandName(document::data::CmdCatalogue &worker, int row, const QVariant &value);
    bool setCommandType(document::data::CmdCatalogue &worker, int row, const QVariant &value);
    bool setCommandVerbose(document::data::CmdCatalogue &worker, int row, const QVariant &value);
    bool setCommandIrAddress(document::data::CmdCatalogue &worker, int row, const QVariant &value);
    bool setCommandIrCommand(document::data::CmdCatalogue &worker, int row, const QVariant &value);
    bool setCommandIrBits(document::data::CmdCatalogue &worker, int row, const QVariant &value);
    bool setCommandData(document::data::CmdCatalogue &worker, int row, const QVariant &value);
};

}

