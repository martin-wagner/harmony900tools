// SPDX-License-Identifier: LGPL-2.1-or-later

#include "models/base.h"
#include "combobox.h"

#include <QComboBox>
#include <QTimer>

namespace delegates
{

ComboBox::ComboBox(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

ComboBox::~ComboBox() = default;

QWidget* ComboBox::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto *comboBox = new QComboBox(parent);

  const QVariantList selectionItems = index.data(
      models::UserDataRole::SelectionItemsRole).toList();

  for (const QVariant &item : selectionItems) {
    comboBox->addItem(item.toString());
  }

  //workaround, immediately show the editor
  QTimer::singleShot(0, comboBox, &QComboBox::showPopup);

  return comboBox;
}

void ComboBox::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  auto *comboBox = qobject_cast<QComboBox*>(editor);
  if (comboBox == nullptr) {
    return;
  }

  const QString currentValue = index.data(Qt::EditRole).toString();
  const int comboIndex = comboBox->findText(currentValue);
  if (comboIndex >= 0) {
    comboBox->setCurrentIndex(comboIndex);
  }
}

void ComboBox::setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const
{
  auto *comboBox = qobject_cast<QComboBox*>(editor);
  if (comboBox == nullptr) {
    return;
  }

  model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void ComboBox::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}

}
