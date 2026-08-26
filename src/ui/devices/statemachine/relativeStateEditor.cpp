// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>

#include "lib/icon.h"
#include "lib/qtHelpers.h"
#include "document/config.h"
#include "relativeStateEditor.h"
#include "deviceActionEditor.h"

using namespace std;
using namespace document::data::item;

namespace editors
{

RelativeStateEditor::RelativeStateEditor(Context &ctx, QWidget *parent) :
    QWidget(parent), config(*ctx.config())
{
  createView(ctx);
  createConnections();
}

void RelativeStateEditor::setRelativeActions(uint32_t devicePos, uint32_t smPos)
{
  this->devicePos = devicePos;
  this->smPos = smPos;

  updateData();
}

void RelativeStateEditor::updateData()
{
  auto &actions = getActions();

  statesList->clear();

  if ((devicePos == 0xffffffff) || (smPos == 0xffffffff)) {
    return;
  }

  for (const string &state : actions.states) {
    QListWidgetItem *item = new QListWidgetItem(qstr(state), statesList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }

  //todo nextAction = relativeActions.nextStateAction.value_or(DeviceAction { });
  updateActionButtonLabel(nextActionButton, nextAction);

  //todo prevAction = relativeActions.prevStateAction;
  updatePrevSlotState();

  //todo resetAction = relativeActions.resetAction;
  updateResetSlotState();
}

void RelativeStateEditor::onAddStateClicked()
{
  config.modify().addDeviceSmStateCommand(devicePos, smPos,
      StateMachineType::Relative, makeStateNameUnique(tr("New State")), -1);
}

void RelativeStateEditor::onRemoveStateClicked()
{
  auto &actions = getActions();

  auto item = statesList->currentRow();
  if ((item < 0) || (item >= actions.states.size())) {
    return;
  }
  config.modify().removeDeviceSmStateCommand(devicePos, smPos,
      StateMachineType::Relative, item);
}

void RelativeStateEditor::onEditNextActionClicked()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Edit next action"));

  DeviceActionEditor *editor = new DeviceActionEditor(&dialog);
  editor->setTitle(tr("Next action"));
  editor->setDeviceAction(nextAction);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  layout->addWidget(editor);
  layout->addWidget(buttonBox);

  if (dialog.exec() == QDialog::Accepted) {
    nextAction = editor->getDeviceAction();
    updateActionButtonLabel(nextActionButton, nextAction);
  }
}

void RelativeStateEditor::onEditPrevActionClicked()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Edit previous action"));

  DeviceActionEditor *editor = new DeviceActionEditor(&dialog);
  editor->setTitle(tr("Prev action"));
  editor->setDeviceAction(prevAction.value_or(DeviceAction { }));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  layout->addWidget(editor);
  layout->addWidget(buttonBox);

  if (dialog.exec() == QDialog::Accepted) {
    prevAction = editor->getDeviceAction();
    updatePrevSlotState();
  }
}

void RelativeStateEditor::onEditResetActionClicked()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Edit reset action"));

  DeviceActionEditor *editor = new DeviceActionEditor(&dialog);
  editor->setTitle(tr("Reset action"));
  editor->setDeviceAction(resetAction.value_or(DeviceAction { }));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  layout->addWidget(editor);
  layout->addWidget(buttonBox);

  if (dialog.exec() == QDialog::Accepted) {
    resetAction = editor->getDeviceAction();
    updateResetSlotState();
  }
}

void RelativeStateEditor::onClearPrevActionClicked()
{
  prevAction = nullopt;
  updatePrevSlotState();
}

void RelativeStateEditor::onClearResetActionClicked()
{
  resetAction = nullopt;
  updateResetSlotState();
}

void RelativeStateEditor::onStateNameChanged(int row, const QString &text)
{
  if (text.toStdString() == getActions().states.at(row)) {
    return; //not changed
  }
  auto name = makeStateNameUnique(text);
  config.modify().setDeviceSmStateName(name, devicePos, smPos,
      StateMachineType::Relative, row);
}

void RelativeStateEditor::onStateNameMoved(int start, int destinationRow)
{
  int i;

  if (destinationRow > start) {
    destinationRow--;
  }
  auto states = getActions().states;
  if (start < destinationRow) {
    rotate(states.begin() + start, states.begin() + start + 1,
        states.begin() + destinationRow + 1);
  } else if (start > destinationRow) {
    rotate(states.begin() + destinationRow, states.begin() + start,
        states.begin() + start + 1);
  } else {
    //nothing to do
    return;
  }

  config.beginMacro(tr("Move State"));

  for (i = 0; i < states.size(); i++) {
    config.modify().setDeviceSmStateName(qstr(states[i]), devicePos, smPos,
        StateMachineType::Relative, i);
  }

  config.endMacro();
}

const document::data::item::RelativeActions& RelativeStateEditor::getActions() const
{
  return config.data().getDevices().at(devicePos).getStateMachines().at(smPos).relative;
}

void RelativeStateEditor::createView(Context &ctx)
{
  statesList = new QListWidget(this);
  statesList->setFlow(QListView::LeftToRight);
  statesList->setWrapping(true);
  statesList->setDragDropMode(QAbstractItemView::InternalMove);
  statesList->setMaximumHeight(80);

  addStateButton = new QPushButton(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/list-add.png",
          "list-add"), "", this);
  removeStateButton = new QPushButton(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
          "edit-delete"), "", this);

  QVBoxLayout *statesLayout = new QVBoxLayout();
  QLabel *statesLabel = new QLabel(
      tr("States, in cycle order (drag to reorder)"), this);
  statesLayout->addWidget(statesLabel);

  QHBoxLayout *statesRowLayout = new QHBoxLayout();
  statesRowLayout->addWidget(statesList, 1);
  QVBoxLayout *buttonAddDeleteLayout = new QVBoxLayout();
  buttonAddDeleteLayout->addWidget(addStateButton, 0, Qt::AlignTop);
  buttonAddDeleteLayout->addWidget(removeStateButton, 0, Qt::AlignTop);
  statesRowLayout->addLayout(buttonAddDeleteLayout);
  statesLayout->addLayout(statesRowLayout);

  nextActionButton = new QPushButton(this);
  connect(nextActionButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onEditNextActionClicked);
  QGroupBox *nextGroup = new QGroupBox(tr("Next action"), this);
  QVBoxLayout *nextLayout = new QVBoxLayout(nextGroup);
  nextLayout->addWidget(nextActionButton);

  prevActionButton = new QPushButton(this);
  connect(prevActionButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onEditPrevActionClicked);
  prevActionClearButton = new QPushButton(tr("Clear"), this);
  connect(prevActionClearButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onClearPrevActionClicked);
  QGroupBox *prevGroup = new QGroupBox(tr("Prev action (optional)"), this);
  QHBoxLayout *prevLayout = new QHBoxLayout(prevGroup);
  prevLayout->addWidget(prevActionButton, 1);
  prevLayout->addWidget(prevActionClearButton, 0);

  resetActionButton = new QPushButton(this);
  connect(resetActionButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onEditResetActionClicked);
  resetActionClearButton = new QPushButton(tr("Clear"), this);
  connect(resetActionClearButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onClearResetActionClicked);
  QGroupBox *resetGroup = new QGroupBox(
      tr("Reset action (optional, fires on PowerOn)"), this);
  QHBoxLayout *resetLayout = new QHBoxLayout(resetGroup);
  resetLayout->addWidget(resetActionButton, 1);
  resetLayout->addWidget(resetActionClearButton, 0);

  QHBoxLayout *actionsRowLayout = new QHBoxLayout();
  actionsRowLayout->addWidget(nextGroup);
  actionsRowLayout->addWidget(prevGroup);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addLayout(statesLayout);
  rootLayout->addLayout(actionsRowLayout);
  rootLayout->addWidget(resetGroup);
  rootLayout->addStretch(1);
}

void RelativeStateEditor::createConnections()
{
  connect(addStateButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onAddStateClicked);
  connect(removeStateButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onRemoveStateClicked);
  connect(statesList, &QListWidget::itemChanged, this,
      [this](QListWidgetItem *item) {
        if (item != nullptr) {
          onStateNameChanged(statesList->row(item), item->text());
        }
      });
  connect(statesList->model(), &QAbstractItemModel::rowsMoved, this,
      [this](const QModelIndex&, int start, int, const QModelIndex&,
          int destinationRow) {
            onStateNameMoved(start, destinationRow);
          });
}

void RelativeStateEditor::updatePrevSlotState()
{
//  const bool hasAction = prevAction.has_value();
//  prevActionButton->setText(
//      hasAction ?
//          tr("%n step(s)", "", static_cast<int>(prevAction->size())) :
//          tr("Not set"));
//  prevActionClearButton->setEnabled(hasAction);
}

void RelativeStateEditor::updateResetSlotState()
{
//  const bool hasAction = resetAction.has_value();
//  resetActionButton->setText(
//      hasAction ?
//          tr("%n step(s)", "", static_cast<int>(resetAction->size())) :
//          tr("Not set"));
//  resetActionClearButton->setEnabled(hasAction);
}

void RelativeStateEditor::updateActionButtonLabel(QPushButton *button,
    const DeviceAction &deviceAction)
{
//  button->setText(tr("%n step(s)", "", static_cast<int>(deviceAction.size())));
}

QString RelativeStateEditor::makeStateNameUnique(const QString &name)
{
  auto usedNames = lib::toQStringList(getActions().states);
  return lib::makeStringUnique(usedNames, name);
}

}
