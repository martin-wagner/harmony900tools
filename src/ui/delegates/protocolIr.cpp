// SPDX-License-Identifier: LGPL-2.1-or-later

#include "protocolIr.h"
#include "ui/editors/codeEditor.h"

#include "bin/irProto/code.h"
#include "document/data/enum.h"
#include "models/protocolIrListModel.h"

#include <QAbstractItemModel>

using namespace document::data;

namespace delegates
{

ProtocolIr::ProtocolIr(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

ProtocolIr::~ProtocolIr() = default;

QWidget* ProtocolIr::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto typeIndex = index.sibling(index.row(),
      models::ProtocolIrModel::Column::TYPE);
  auto codeTypeStr = typeIndex.data(Qt::EditRole).toString();
  auto codeType = Enum<CodeType>(codeTypeStr);

  if ((codeType.getValue() != CodeType::Proprietary)
      && (codeType.getValue() != CodeType::PhilipsRc5)) {
    //unsupported protocol, no editor available yet
    return nullptr;
  } //todo hier wegmachen. eine stelle wo das steht sollte ausreichen.

  auto code = index.data(Qt::EditRole).value<binary::irProto::Code>();

  editors::CodeEditor editor(code, codeType.getValue(), parent);
  if (editor.exec() == QDialog::Accepted) {
    auto *model = const_cast<QAbstractItemModel*>(index.model());
    model->setData(index, QVariant::fromValue(editor.getCode()), Qt::EditRole);
  }

  //no inline editor widget is used, dialog already handled everything
  return nullptr;
}

}
