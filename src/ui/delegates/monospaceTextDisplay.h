// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStyledItemDelegate>

namespace delegates
{

/**
 * @brief Delegate that displays DisplayRole as monospace text.
 *
 * DisplayRole is expected to be a QString, for example an ascii-art
 * representation of some raw data. EditRole is left untouched and can
 * carry any type, since this delegate never reads or writes it.
 *
 * The delegate is display-only: no editor is ever created, so the
 * user cannot edit the cell. Selecting the cell does not change any
 * data; if something needs to be triggered on selection, or the data
 * needs to be set programmatically, this is done outside the delegate.
 */
class MonospaceTextDisplay: public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit MonospaceTextDisplay(QObject *parent = nullptr);
    ~MonospaceTextDisplay() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
