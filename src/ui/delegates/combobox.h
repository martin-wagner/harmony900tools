// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStyledItemDelegate>

namespace delegates
{

/**
 * @brief Delegate that edits a cell using a QComboBox.
 *
 * The available options are pulled from the model via
 * UserDataRole::SelectionItemsRole, expected to be a QVariantList
 * of QStrings.
 */
class ComboBox: public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit ComboBox(QObject *parent = nullptr);
    ~ComboBox() override;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
