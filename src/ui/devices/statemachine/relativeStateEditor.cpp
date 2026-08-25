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

#include "relativeStateEditor.h"
#include "deviceActionEditor.h"

using namespace document::data::item;

namespace editors
{

RelativeStateEditor::RelativeStateEditor(QWidget *parent) :
    QWidget(parent)
{
  buildUi();
}

void RelativeStateEditor::setRelativeActions(
    const RelativeActions &relativeActions)
{
  statesList->clear();
  for (const std::string &state : relativeActions.states) {
    QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(state),
        statesList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }

  nextAction = relativeActions.nextStateAction.value_or(DeviceAction { });
  updateActionButtonLabel(nextActionButton, nextAction);

  prevAction = relativeActions.prevStateAction;
  updatePrevSlotState();

  resetAction = relativeActions.resetAction;
  updateResetSlotState();
}

RelativeActions RelativeStateEditor::getRelativeActions() const
{
  RelativeActions relativeActions;

  for (int i = 0; i < statesList->count(); ++i) {
    relativeActions.states.push_back(statesList->item(i)->text().toStdString());
  }

  relativeActions.nextStateAction = nextAction;
  relativeActions.prevStateAction = prevAction;
  relativeActions.resetAction = resetAction;

  return relativeActions;
}

void RelativeStateEditor::onAddStateClicked()
{
  bool ok = false;
  const QString stateName = QInputDialog::getText(this, tr("Add state"),
      tr("State value"), QLineEdit::Normal, tr("NewValue"), &ok);

  if (!ok || stateName.isEmpty()) {
    return;
  }

  QListWidgetItem *item = new QListWidgetItem(stateName, statesList);
  item->setFlags(item->flags() | Qt::ItemIsEditable);

  emit changed();
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
    emit changed();
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
    emit changed();
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
    emit changed();
  }
}

void RelativeStateEditor::onClearPrevActionClicked()
{
  prevAction = std::nullopt;
  updatePrevSlotState();
  emit changed();
}

void RelativeStateEditor::onClearResetActionClicked()
{
  resetAction = std::nullopt;
  updateResetSlotState();
  emit changed();
}

void RelativeStateEditor::buildUi()
{
  statesList = new QListWidget(this);
  statesList->setFlow(QListView::LeftToRight);
  statesList->setWrapping(true);
  statesList->setDragDropMode(QAbstractItemView::InternalMove);
  statesList->setMaximumHeight(80);

  addStateButton = new QPushButton(tr("Add state"), this);
  connect(addStateButton, &QPushButton::clicked, this,
      &RelativeStateEditor::onAddStateClicked);

  QVBoxLayout *statesLayout = new QVBoxLayout();
  QLabel *statesLabel = new QLabel(
      tr("States, in cycle order (drag to reorder)"), this);
  statesLayout->addWidget(statesLabel);

  QHBoxLayout *statesRowLayout = new QHBoxLayout();
  statesRowLayout->addWidget(statesList, 1);
  statesRowLayout->addWidget(addStateButton, 0, Qt::AlignTop);
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

}
