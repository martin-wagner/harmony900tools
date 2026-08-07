// SPDX-License-Identifier: LGPL-2.1-or-later

#include "rawIr.h"
#include "ui/editors/rawIrEditor.h"

#include <QAbstractItemModel>

namespace delegates
{

RawIr::RawIr(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

RawIr::~RawIr() = default;

QWidget* RawIr::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto stream = index.data(Qt::EditRole).value<binary::ssIr::SerialStreamIr>();

  editors::RawIrEditor editor(stream, parent);
  if (editor.exec() == QDialog::Accepted) {
    auto *model = const_cast<QAbstractItemModel*>(index.model());
    model->setData(index, QVariant::fromValue(editor.getStream()), Qt::EditRole);
  }

  //no inline editor widget is used, dialog already handled everything
  return nullptr;
}

}
