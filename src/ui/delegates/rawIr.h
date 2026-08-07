// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStyledItemDelegate>

namespace delegates
{

/**
 * @brief Delegate editing binary::ssIr::SerialStreamIr
 *
 * Display/tooltip/font behaviour is unchanged (inherited from
 * QStyledItemDelegate). Double-clicking (or any other configured edit
 * trigger) opens a modal RawIrEditor dialog instead of an inline editor.
 */
class RawIr: public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit RawIr(QObject *parent = nullptr);
    ~RawIr() override;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
