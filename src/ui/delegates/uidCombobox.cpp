// SPDX-License-Identifier: LGPL-2.1-or-later

#include "models/base.h"
#include "uidCombobox.h"

#include <QComboBox>
#include <QTimer>

namespace delegates
{

UidComboBox::UidComboBox(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

UidComboBox::~UidComboBox() = default;

QWidget* UidComboBox::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto *comboBox = new QComboBox(parent);

  auto selectionItems =
      index.data(models::UserDataRole::SelectionItemsRole).value<
          QList<QPair<uint32_t, QString>>>();

  for (const auto &item : selectionItems) {
    comboBox->addItem(item.second);
  }

  //workaround, immediately show the editor
  QTimer::singleShot(0, comboBox, &QComboBox::showPopup);

  return comboBox;
}

void UidComboBox::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  auto *comboBox = qobject_cast<QComboBox*>(editor);
  if (comboBox == nullptr) {
    return;
  }

  const QString currentValue = index.data(Qt::DisplayRole).toString();
  const int comboIndex = comboBox->findText(currentValue);
  if (comboIndex >= 0) {
    comboBox->setCurrentIndex(comboIndex);
  }
}

void UidComboBox::setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const
{
  auto *comboBox = qobject_cast<QComboBox*>(editor);
  if (comboBox == nullptr) {
    return;
  }

  auto selectionItems =
      index.data(models::UserDataRole::SelectionItemsRole).value<
          QList<QPair<uint32_t, QString>>>();
  for (const auto &item : selectionItems) {
    if (comboBox->currentText() == item.second) {
      model->setData(index, item.first, Qt::EditRole);
      return;
    }
  }
  //fixme error out
}

void UidComboBox::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}

}
