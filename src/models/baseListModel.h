// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/catalogue.h"

namespace document
{
  class Config;
}

namespace models
{

//Some common functions for list models
class BaseModel: public QAbstractItemModel
{
  Q_OBJECT
  public:
    BaseModel(document::data::Item item, QObject *parent = nullptr);
    ~BaseModel() override;

    QVariant data(const QModelIndex &index, int role) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns, const QModelIndex &parent = { }) override;
    bool removeColumns(int position, int columns, const QModelIndex &parent = { }) override;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    document::data::Item item;

    void createActions(document::Config *config);

  protected:
    virtual QVariant getDisplayData(const QModelIndex &index) const;
    virtual QVariant getEditData(const QModelIndex &index) const;
    virtual QVariant getCheckStateData(const QModelIndex &index) const;
    virtual QVariant getTooltipData(const QModelIndex &index) const;
    virtual QVariant getBackgroundData(const QModelIndex &index) const;
    virtual QVariant getForegroundData(const QModelIndex &index) const;
    virtual QVariant getFontData(const QModelIndex &index) const;
    virtual QVariant getSelectionItemsData(const QModelIndex &index) const;

    QString makeStringUnique(const QStringList &input, QString str);
    static QStringList toQStringList(const std::vector<std::string> &list);
    static std::vector<std::string> toStringList(const QStringList &qlist);

  protected slots:
    virtual void itemChangedObserver(document::data::Item item, int pos);
    virtual void itemAboutToBeAddedObserver(document::data::Item item, int pos);
    virtual void itemAddedObserver(document::data::Item item, int pos);
    virtual void itemAboutToBeRemovedObserver(document::data::Item item, int pos);
    virtual void itemRemovedObserver(document::data::Item item, int pos);
};

}

