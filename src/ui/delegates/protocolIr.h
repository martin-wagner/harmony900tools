// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStyledItemDelegate>

namespace delegates
{

/**
 * @brief Delegate editing binary::irProto::Code.
 *
 * Display/tooltip/font behaviour is unchanged (inherited from
 * QStyledItemDelegate). Double-clicking (or any other configured edit
 * trigger) opens a modal CodeEditor dialog instead of an inline editor.
 * The dialog's fields depend on the CodeType read from Column::TYPE on
 * the same row.
 */
class ProtocolIr: public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit ProtocolIr(QObject *parent = nullptr);
    ~ProtocolIr() override;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
