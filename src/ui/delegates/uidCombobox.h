// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStyledItemDelegate>

namespace delegates
{

/**
 * @brief Delegate that edits a cell using a QUidComboBox.
 *
 * The available options are pulled from the model via
 * UserDataRole::SelectionItemsRole, expected to be a QVariantList
 * of QPair<uint32_t, QString>, where uid contains a unique value
 * identifying the string.
 */
class UidComboBox: public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit UidComboBox(QObject *parent = nullptr);
    ~UidComboBox() override;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
