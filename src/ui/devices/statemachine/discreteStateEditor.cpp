// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>

#include "discreteStateEditor.h"
#include "deviceActionEditor.h"

using namespace document::data::item;

namespace editors
{

constexpr int ColumnStateName = 0;
constexpr int ColumnAction = 1;
constexpr int ColumnRemove = 2;

const char *PropertyDeviceAction = "deviceAction";

DiscreteStateEditor::DiscreteStateEditor(QWidget *parent) :
    QWidget(parent)
{
  buildUi();
}

void DiscreteStateEditor::setDiscreteActions(
    const DiscreteActions &discreteActions)
{
  table->setRowCount(0);

  for (std::size_t i = 0; i < discreteActions.states.size(); ++i) {
    const QString stateName = QString::fromStdString(discreteActions.states[i]);
    const DeviceAction enterAction =
        i < discreteActions.enterStateAction.size() ?
            discreteActions.enterStateAction[i] : DeviceAction { };
    addStateRow(stateName, enterAction);
  }
}

DiscreteActions DiscreteStateEditor::getDiscreteActions() const
{
  DiscreteActions discreteActions;

  for (int row = 0; row < table->rowCount(); ++row) {
    QLineEdit *nameEdit = qobject_cast<QLineEdit*>(
        table->cellWidget(row, ColumnStateName));
    QPushButton *actionButton = qobject_cast<QPushButton*>(
        table->cellWidget(row, ColumnAction));

    if (nameEdit == nullptr || actionButton == nullptr) {
      continue;
    }

    discreteActions.states.push_back(nameEdit->text().toStdString());
    discreteActions.enterStateAction.push_back(
        actionButton->property(PropertyDeviceAction).value<DeviceAction>());
  }

  return discreteActions;
}

void DiscreteStateEditor::onAddStateClicked()
{
  addStateRow(tr("NewValue"), DeviceAction { });
  emit changed();
}

void DiscreteStateEditor::onRemoveStateClicked()
{
  QPushButton *button = qobject_cast<QPushButton*>(sender());
  if (button == nullptr) {
    return;
  }

  for (int row = 0; row < table->rowCount(); ++row) {
    if (table->cellWidget(row, ColumnRemove) == button) {
      table->removeRow(row);
      break;
    }
  }

  emit changed();
}

void DiscreteStateEditor::onActionButtonClicked()
{
  QPushButton *actionButton = qobject_cast<QPushButton*>(sender());
  if (actionButton == nullptr) {
    return;
  }

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
    emit changed();
  }
}

void DiscreteStateEditor::buildUi()
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
  connect(addStateButton, &QPushButton::clicked, this,
      &DiscreteStateEditor::onAddStateClicked);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addWidget(table);
  rootLayout->addWidget(addStateButton);
}

void DiscreteStateEditor::addStateRow(const QString &stateName,
    const DeviceAction &enterAction)
{
  const int row = table->rowCount();
  table->insertRow(row);

  QLineEdit *nameEdit = new QLineEdit(stateName, table);
  table->setCellWidget(row, ColumnStateName, nameEdit);

  QPushButton *actionButton = new QPushButton(table);
  actionButton->setProperty(PropertyDeviceAction,
      QVariant::fromValue(enterAction));
  updateActionButtonLabel(actionButton, enterAction);
  connect(actionButton, &QPushButton::clicked, this,
      &DiscreteStateEditor::onActionButtonClicked);
  table->setCellWidget(row, ColumnAction, actionButton);

  QPushButton *removeButton = new QPushButton(QStringLiteral("\u00D7"), table);
  removeButton->setToolTip(tr("Remove this value"));
  connect(removeButton, &QPushButton::clicked, this,
      &DiscreteStateEditor::onRemoveStateClicked);
  table->setCellWidget(row, ColumnRemove, removeButton);
}

void DiscreteStateEditor::updateActionButtonLabel(QPushButton *button,
    const DeviceAction &deviceAction)
{
  //button->setText(tr("%n step(s)", "", static_cast<int>(deviceAction.size())));
}

}
