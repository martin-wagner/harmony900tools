// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAbstractItemModel>

#include "protocolIr.h"
#include "ui/editors/codeEditor.h"
#include "bin/irProto/code.h"
#include "document/data/enum.h"
#include "models/protocolIrListModel.h"
#include "context.h"

using namespace document::data;

namespace delegates
{

ProtocolIr::ProtocolIr(Context &ctx, QObject *parent) :
    QStyledItemDelegate(parent), ctx(ctx)
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

  if (!editors::CodeEditor::isSupported(codeType.getValue())) {
    //unsupported protocol, no editor available yet
    return nullptr;
  }

  auto code = index.data(Qt::EditRole).value<binary::irProto::Code>();

  editors::CodeEditor editor(ctx, code, codeType.getValue(), parent);
  if (editor.exec() == QDialog::Accepted) {
    auto *model = const_cast<QAbstractItemModel*>(index.model());
    model->setData(index, QVariant::fromValue(editor.getCode()), Qt::EditRole);
  }

  //no inline editor widget is used, dialog already handled everything
  return nullptr;
}

}
