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
}

void RelativeStateEditor::updateData()
{
  statesList->clear();

  if ((devicePos == 0xffffffff) || (smPos == 0xffffffff)) {
    return;
  }
  auto &actions = getActions();

  for (const string &state : actions.states) {
    QListWidgetItem *item = new QListWidgetItem(qstr(state), statesList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }

  if (actions.nextStateAction.has_value()) {
    nextActionButton->setText(
        tr("%n step(s)", "",
            static_cast<int>(actions.nextStateAction->sequence.size())));
    nextActionButton->setStyleSheet("");
  } else {
    nextActionButton->setText(tr("Add"));
    nextActionButton->setStyleSheet("QPushButton {"
        "    background-color: red;"
        "    color: white;"
        "}");
  }
  if (actions.prevStateAction.has_value()) {
    prevActionButton->setText(
        tr("%n step(s)", "",
            static_cast<int>(actions.prevStateAction->sequence.size())));
  } else {
    prevActionButton->setText(tr("Add"));
  }
  if (actions.resetAction.has_value()) {
    resetActionButton->setText(
        tr("%n step(s)", "",
            static_cast<int>(actions.resetAction->sequence.size())));
  } else {
    resetActionButton->setText(tr("Add"));
  }
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
  auto a = getActions().nextStateAction.value_or(DeviceAction());

  auto data = DeviceActionEditor::openEditor(a,
      document::data::ActionType::NextAction, tr("Transition to next state"),
      this);
  if (!data.has_value()) {
    return;
  }

  config.beginMacro("Edit Next Action");

  if (!getActions().nextStateAction.has_value()) {
    config.modify().addDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Relative_Next);
  }
  config.modify().setDeviceSmAction(data.value(), devicePos, smPos,
      StateMachineAction::Relative_Next, 0);

  config.endMacro();
}

void RelativeStateEditor::onEditPrevActionClicked()
{
  auto a = getActions().prevStateAction.value_or(DeviceAction());

  auto data = DeviceActionEditor::openEditor(a,
      document::data::ActionType::PrevAction,
      tr("Transition to previous state"), this);
  if (!data.has_value()) {
    return;
  }

  config.beginMacro("Edit Previous Action");

  if (!getActions().prevStateAction.has_value()) {
    config.modify().addDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Relative_Prev);
  }
  config.modify().setDeviceSmAction(data.value(), devicePos, smPos,
      StateMachineAction::Relative_Prev, 0);

  config.endMacro();
}

void RelativeStateEditor::onEditResetActionClicked()
{
  auto a = getActions().resetAction.value_or(DeviceAction());

  auto data = DeviceActionEditor::openEditor(a,
      document::data::ActionType::ResetAction,
      tr("Transition to initial state"), this);
  if (!data.has_value()) {
    return;
  }

  config.beginMacro("Edit Reset Action");

  if (!getActions().resetAction.has_value()) {
    config.modify().addDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Relative_Reset);
  }
  config.modify().setDeviceSmAction(data.value(), devicePos, smPos,
      StateMachineAction::Relative_Reset, 0);

  config.endMacro();
}

void RelativeStateEditor::onClearNextActionClicked()
{
  if (getActions().nextStateAction.has_value()) {
    config.beginMacro("Remove Next Action");
    config.modify().removeDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Relative_Next);
    config.endMacro();
  }
}

void RelativeStateEditor::onClearPrevActionClicked()
{
  if (getActions().prevStateAction.has_value()) {
    config.beginMacro("Remove Previous Action");
    config.modify().removeDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Relative_Prev);
    config.endMacro();
  }
}

void RelativeStateEditor::onClearResetActionClicked()
{
  if (getActions().resetAction.has_value()) {
    config.beginMacro("Remove Reset Action");
    config.modify().removeDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Relative_Reset);
    config.endMacro();
  }
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

const RelativeActions& RelativeStateEditor::getActions() const
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

  addStateButton = new QPushButton(lib::getAddIcon(), "", this);
  removeStateButton = new QPushButton(lib::getDeleteIcon(), "", this);

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

  nextActionButton = new QPushButton(lib::getEditIcon(), tr("Add"), this);
  nextActionClearButton = new QPushButton(lib::getDeleteIcon(), "", this);
  QGroupBox *nextGroup = new QGroupBox(tr("Next action (mandatory)"), this);
  nextGroup->setToolTip("This cycles to the next state (e.g. \"source\" or "
      "\"+\" button).\nThe control will not work if this is not set!");
  QHBoxLayout *nextLayout = new QHBoxLayout(nextGroup);
  nextLayout->addWidget(nextActionButton, 1);
  nextLayout->addWidget(nextActionClearButton, 0);

  prevActionButton = new QPushButton(lib::getEditIcon(), tr("Add"), this);
  prevActionClearButton = new QPushButton(lib::getDeleteIcon(), "", this);
  QGroupBox *prevGroup = new QGroupBox(tr("Previous action (optional)"), this);
  prevGroup->setToolTip("This cycles to the previous state (e.g. \"-\" "
      "button). \nIf available, next and previous will be used for cycling "
      "trough the states.");
  QHBoxLayout *prevLayout = new QHBoxLayout(prevGroup);
  prevLayout->addWidget(prevActionButton, 1);
  prevLayout->addWidget(prevActionClearButton, 0);

  resetActionButton = new QPushButton(lib::getEditIcon(), tr("Add"), this);
  resetActionClearButton = new QPushButton(lib::getDeleteIcon(), "", this);
  QGroupBox *resetGroup = new QGroupBox(tr("Reset action (optional)"), this);
  resetGroup->setToolTip("This resets your device to the start position\n"
      "You device liklely doesn't support this.");
  QHBoxLayout *resetLayout = new QHBoxLayout(resetGroup);
  resetLayout->addWidget(resetActionButton, 1);
  resetLayout->addWidget(resetActionClearButton, 0);

  QHBoxLayout *actionLayout = new QHBoxLayout();
  actionLayout->addWidget(nextGroup, 1);
  actionLayout->addWidget(prevGroup, 1);
  actionLayout->addWidget(resetGroup, 1);
  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addLayout(statesLayout);
  rootLayout->addLayout(actionLayout);
  rootLayout->addStretch(1);
}

void RelativeStateEditor::createConnections()
{
  connect(nextActionButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onEditNextActionClicked);
  connect(nextActionClearButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onClearNextActionClicked);
  connect(prevActionButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onEditPrevActionClicked);
  connect(prevActionClearButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onClearPrevActionClicked);
  connect(resetActionButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onEditResetActionClicked);
  connect(resetActionClearButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onClearResetActionClicked);
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

QString RelativeStateEditor::makeStateNameUnique(const QString &name)
{
  auto usedNames = lib::toQStringList(getActions().states);
  return lib::makeStringUnique(usedNames, name);
}

}
