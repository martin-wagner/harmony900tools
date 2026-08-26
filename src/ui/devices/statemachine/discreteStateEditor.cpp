// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>

#include "lib/icon.h"
#include "lib/qtHelpers.h"
#include "document/config.h"
#include "discreteStateEditor.h"
#include "deviceActionEditor.h"

using namespace std;
using namespace document::data::item;

namespace editors
{

constexpr int ColumnStateName = 0;
constexpr int ColumnAction = 1;
constexpr int ColumnRemove = 2;

const char *PropertyDeviceAction = "deviceAction";

DiscreteStateEditor::DiscreteStateEditor(Context &ctx, QWidget *parent) :
    QWidget(parent), config(*ctx.config())
{
  createView(ctx);
  createConnections();
}

void DiscreteStateEditor::setDiscreteActions(uint32_t devicePos, uint32_t smPos)
{
  this->devicePos = devicePos;
  this->smPos = smPos;

  auto &actions = getActions();
  if (actions.states.size() != actions.enterStateAction.size()) {
    //invalid
    return;
  }

  updateData();
}

void DiscreteStateEditor::updateData()
{
  int i;

  //fixme brute force. maybe use model/view instead?

  table->setRowCount(0);

  if ((devicePos == 0xffffffff) || (smPos == 0xffffffff)) {
    return;
  }

  auto &actions = getActions();
  for (i = 0; i < actions.states.size(); i++) {
    auto stateName = qstr(actions.states[i]);
    addStateRow(stateName, actions.enterStateAction[i]);
  }
}

void DiscreteStateEditor::onAddStateClicked()
{
  config.modify().addDeviceSmStateCommand(devicePos, smPos,
      StateMachineType::Discrete, makeStateNameUnique(tr("New State")), -1);
}

void DiscreteStateEditor::onRemoveStateClicked()
{
  int row;

  QPushButton *button = qobject_cast<QPushButton*>(sender());
  if (button == nullptr) {
    return;
  }

  for (row = 0; row < table->rowCount(); row++) {
    if (table->cellWidget(row, ColumnRemove) == button) {
      config.modify().removeDeviceSmStateCommand(devicePos, smPos,
          StateMachineType::Discrete, row);
      break;
    }
  }
}

void DiscreteStateEditor::onActionButtonClicked()
{
  QPushButton *actionButton = qobject_cast<QPushButton*>(sender());
  if (actionButton == nullptr) {
    return;
  }

  //todo use action editor

  int row = -1;
  for (int r = 0; r < table->rowCount(); ++r) {
    if (table->cellWidget(r, ColumnAction) == actionButton) {
      row = r;
      break;
    }
  }
  if (row < 0) {
    return;
  }

  QLineEdit *nameEdit = qobject_cast<QLineEdit*>(
      table->cellWidget(row, ColumnStateName));
  const QString stateName = nameEdit != nullptr ? nameEdit->text() : QString();

  QDialog dialog(this);
  dialog.setWindowTitle(tr("Edit action"));

  DeviceActionEditor *editor = new DeviceActionEditor(&dialog);
  editor->setTitle(tr("Enter action - state %1").arg(stateName));
  editor->setDeviceAction(
      actionButton->property(PropertyDeviceAction).value<DeviceAction>());

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  layout->addWidget(editor);
  layout->addWidget(buttonBox);

  if (dialog.exec() == QDialog::Accepted) {
    const DeviceAction deviceAction = editor->getDeviceAction();
    actionButton->setProperty(PropertyDeviceAction,
        QVariant::fromValue(deviceAction));
    updateActionButtonLabel(actionButton, deviceAction);
  }
}

void DiscreteStateEditor::onStateNameChanged(int row, const QString &text)
{
  if (text.toStdString() == getActions().states.at(row)) {
    return; //not changed
  }
  auto name = makeStateNameUnique(text);
  config.modify().setDeviceSmStateName(name, devicePos, smPos,
      StateMachineType::Discrete, row);
}

const document::data::item::DiscreteActions& DiscreteStateEditor::getActions() const
{
  return config.data().getDevices().at(devicePos).getStateMachines().at(smPos).discrete;
}

void DiscreteStateEditor::createView(Context &ctx)
{
  table = new QTableWidget(0, 3, this);
  table->setHorizontalHeaderLabels(
      { tr("State value"), tr("Action"), QString() });
  table->horizontalHeader()->setSectionResizeMode(ColumnStateName,
      QHeaderView::Stretch);
  table->horizontalHeader()->setSectionResizeMode(ColumnAction,
      QHeaderView::ResizeToContents);
  table->horizontalHeader()->setSectionResizeMode(ColumnRemove,
      QHeaderView::ResizeToContents);
  table->verticalHeader()->setVisible(false);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  addStateButton = new QPushButton(tr("Add value"), this);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addWidget(table);
  rootLayout->addWidget(addStateButton);
}

void DiscreteStateEditor::createConnections()
{
  connect(addStateButton, &QPushButton::clicked, this,
      &DiscreteStateEditor::onAddStateClicked);
}

void DiscreteStateEditor::addStateRow(const QString &stateName,
    const DeviceAction &enterAction)
{
  const int row = table->rowCount();
  table->insertRow(row);

  QLineEdit *nameEdit = new QLineEdit(stateName, table);
  table->setCellWidget(row, ColumnStateName, nameEdit);
  connect(nameEdit, &QLineEdit::editingFinished, this, [this, row, nameEdit]() {
    onStateNameChanged(row, nameEdit->text());
  });

  QPushButton *actionButton = new QPushButton(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/text-field.png",
          "accessories-text-editor"), "", table);
  actionButton->setProperty(PropertyDeviceAction,
      QVariant::fromValue(enterAction));
  updateActionButtonLabel(actionButton, enterAction);
  connect(actionButton, &QPushButton::clicked, this,
      &DiscreteStateEditor::onActionButtonClicked);
  table->setCellWidget(row, ColumnAction, actionButton);

  QPushButton *removeButton = new QPushButton(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
          "edit-delete"), "", table);
  removeButton->setToolTip(tr("Remove this value"));
  connect(removeButton, &QPushButton::clicked, this,
      &DiscreteStateEditor::onRemoveStateClicked);
  table->setCellWidget(row, ColumnRemove, removeButton);
}

void DiscreteStateEditor::updateActionButtonLabel(QPushButton *button,
    const DeviceAction &deviceAction)
{
  button->setText(
      tr("%n step(s)", "", static_cast<int>(deviceAction.sequence.size())));
}

QString DiscreteStateEditor::makeStateNameUnique(const QString &name)
{
  auto usedNames = lib::toQStringList(getActions().states);
  return lib::makeStringUnique(usedNames, name);
}

}
