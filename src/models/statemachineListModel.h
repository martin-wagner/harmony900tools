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

//Model for state machines
class StateMachineModel: public BaseModel
{
  Q_OBJECT
  public:
    enum Column {
      CONTROL_TYPE,
      MACHINE_TYPE,

      COUNT
    };

    const std::map<Column, Setup> columnSetup = {
        { Column::CONTROL_TYPE, { "What", "Which function of your device will be controled.\n"
            "You will most likely never need more than \"Power\" and \"Input\".", "Enum", false, {}, } },
        { Column::MACHINE_TYPE, { "How", "The \"system\" that will be used.\n "
            "- Direct select: You have one separate button for each function\n"
            "- Cycle: You have one button to cycle trough a selection, e.g. \"next\" or \"plus\" and \"minus\"", "QString", false, {}, } },
    };

  public:
    StateMachineModel(document::Config &config, uint32_t deviceId, QObject *parent = nullptr);
    ~StateMachineModel() override;

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

    const std::vector<document::data::item::StateMachine> &getMachines(uint32_t *devicePos = nullptr) const;
    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getSelectionItemsData(const QModelIndex &index) const override;
};

}

