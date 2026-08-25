// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAbstractItemModel>

#include "protocolIr.h"
#include "ui/devices/editors/codeEditor.h"
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
  auto *cmodel = dynamic_cast<const models::ProtocolIrModel*>(index.model());
  if (cmodel == nullptr) {
    //wrong base model
    return nullptr;
  }
  auto *model = const_cast<models::ProtocolIrModel*>(cmodel);

  auto codeTypeStr = model->data(
      index.siblingAtColumn(models::ProtocolIrModel::Column::TYPE),
      Qt::EditRole).toString();
  auto codeType = Enum<CodeType>(codeTypeStr);
  if (!editors::CodeEditor::isSupported(codeType.getValue())) {
    //unsupported protocol, no editor available yet
    return nullptr;
  }
  auto codeString = model->data(
      index.siblingAtColumn(models::ProtocolIrModel::Column::VERBOSE),
      Qt::EditRole).toString();
  auto address = model->data(
      index.siblingAtColumn(models::ProtocolIrModel::Column::IRADDRESS),
      Qt::EditRole).toUInt();
  auto command = model->data(
      index.siblingAtColumn(models::ProtocolIrModel::Column::IRCOMMAND),
      Qt::EditRole).toUInt();
  auto data = qvariant_cast<std::vector<bool>>(
      model->data(
          index.siblingAtColumn(models::ProtocolIrModel::Column::IRBITS),
          Qt::EditRole));
  auto code = qvariant_cast<binary::irProto::Code>(
      model->data(index.siblingAtColumn(models::ProtocolIrModel::Column::DATA),
          Qt::EditRole));

  editors::CodeEditor editor(ctx, codeType.getValue(), codeString, address,
      command, data, code, parent);

  if (editor.exec() == QDialog::Accepted) {
    ctx.undoStack().beginMacro(
        tr("update command: %1").arg(
            model->data(index.siblingAtColumn(models::ProtocolIrModel::NAME),
                Qt::DisplayRole).toString()));

    model->setData(
        index.siblingAtColumn(models::ProtocolIrModel::Column::VERBOSE),
        editor.getCodeString(), Qt::EditRole);

    model->setData(
        index.siblingAtColumn(models::ProtocolIrModel::Column::IRADDRESS),
        editor.getAddress(), Qt::EditRole);
    model->setData(
        index.siblingAtColumn(models::ProtocolIrModel::Column::IRCOMMAND),
        editor.getCommand(), Qt::EditRole);
    model->setData(
        index.siblingAtColumn(models::ProtocolIrModel::Column::IRBITS),
        QVariant::fromValue(editor.getRawData()), Qt::EditRole);
    model->setData(index.siblingAtColumn(models::ProtocolIrModel::Column::DATA),
        QVariant::fromValue(editor.getCode()), Qt::EditRole);

    ctx.undoStack().endMacro();
  }

  //no inline editor widget is used, dialog already handled everything
  return nullptr;
}

}
