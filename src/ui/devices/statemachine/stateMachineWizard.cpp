// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QWizardPage>
#include <QRadioButton>
#include <QButtonGroup>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>

#include "stateMachineWizard.h"
#include "deviceActionEditor.h"

using namespace document::data::item;

namespace editors
{

StateMachineWizard::StateMachineWizard(QWidget *parent) :
    QWizard(parent)
{
  setWindowTitle(tr("State machine"));
  buildChooseTypePage();
  buildDefineStatesPage();
  buildAssignActionsPage();
  buildReviewPage();

  connect(this, &QWizard::currentIdChanged, this,
      &StateMachineWizard::onPageChanged);
}

void StateMachineWizard::setStateMachine(const StateMachine &stateMachine)
{
  statesList->clear();
  actionsBySlot.clear();

  if (!stateMachine.discrete.empty()) {
    discreteRadio->setChecked(true);

    for (std::size_t i = 0; i < stateMachine.discrete.states.size(); ++i) {
      const QString stateName = QString::fromStdString(
          stateMachine.discrete.states[i]);
      statesList->addItem(stateName);

      const DeviceAction action =
          i < stateMachine.discrete.enterStateAction.size() ?
              stateMachine.discrete.enterStateAction[i] : DeviceAction { };
      actionsBySlot.insert(tr("Set: %1").arg(stateName), action);
    }
  } else {
    relativeRadio->setChecked(true);

    for (const std::string &state : stateMachine.relative.states) {
      statesList->addItem(QString::fromStdString(state));
    }

    actionsBySlot.insert(tr("Next action"),
        stateMachine.relative.nextStateAction.value_or(DeviceAction { }));
    if (stateMachine.relative.prevStateAction.has_value()) {
      actionsBySlot.insert(tr("Prev action"),
          stateMachine.relative.prevStateAction.value());
    }
    if (stateMachine.relative.resetAction.has_value()) {
      actionsBySlot.insert(tr("Reset action"),
          stateMachine.relative.resetAction.value());
    }
  }

  rebuildActionSlots();
}

StateMachine StateMachineWizard::getStateMachine() const
{
  StateMachine stateMachine;

  if (discreteRadio->isChecked()) {
    for (int i = 0; i < statesList->count(); ++i) {
      const QString stateName = statesList->item(i)->text();
      stateMachine.discrete.states.push_back(stateName.toStdString());
      stateMachine.discrete.enterStateAction.push_back(
          actionsBySlot.value(tr("Set: %1").arg(stateName)));
    }
  } else {
    for (int i = 0; i < statesList->count(); ++i) {
      stateMachine.relative.states.push_back(
          statesList->item(i)->text().toStdString());
    }
    stateMachine.relative.nextStateAction = actionsBySlot.value(
        tr("Next action"));
    if (actionsBySlot.contains(tr("Prev action"))) {
      stateMachine.relative.prevStateAction = actionsBySlot.value(
          tr("Prev action"));
    }
    if (actionsBySlot.contains(tr("Reset action"))) {
      stateMachine.relative.resetAction = actionsBySlot.value(
          tr("Reset action"));
    }
  }

  return stateMachine;
}

void StateMachineWizard::onActionSlotChanged(int row)
{
  storeCurrentSlotEdits();

  if (row < 0 || row >= actionSlotsList->count()) {
    currentSlotKey.clear();
    actionEditor->setEnabled(false);
    return;
  }

  currentSlotKey = actionSlotsList->item(row)->text();
  actionEditor->setEnabled(true);
  actionEditor->setTitle(currentSlotKey);
  actionEditor->setDeviceAction(actionsBySlot.value(currentSlotKey));
}

void StateMachineWizard::onAddStateClicked()
{
  bool ok = false;
  const QString stateName = QInputDialog::getText(this, tr("Add state"),
      tr("State value"), QLineEdit::Normal, tr("NewValue"), &ok);

  if (!ok || stateName.isEmpty()) {
    return;
  }

  statesList->addItem(stateName);
  rebuildActionSlots();
}

void StateMachineWizard::onRemoveStateClicked()
{
  QListWidgetItem *item = statesList->currentItem();
  if (item == nullptr) {
    return;
  }

  delete statesList->takeItem(statesList->row(item));
  rebuildActionSlots();
}

void StateMachineWizard::onPageChanged(int id)
{
  if (id == PageAssignActions) {
    rebuildActionSlots();
  } else if (id == PageReview) {
    storeCurrentSlotEdits();
    reviewSummaryLabel->setText(reviewSummaryText());
  }
}

void StateMachineWizard::buildChooseTypePage()
{
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("Choose type"));
  page->setSubTitle(tr("This determines every following step."));

  discreteRadio = new QRadioButton(
      tr("Discrete -- fixed command per state (e.g. On/Off buttons)"), page);
  relativeRadio = new QRadioButton(
      tr("Relative -- only next/prev commands (e.g. power toggle)"), page);
  rangeRadio = new QRadioButton(tr("Range -- typed number (not available yet)"),
      page);
  rangeRadio->setEnabled(false);
  discreteRadio->setChecked(true);

  QButtonGroup *group = new QButtonGroup(page);
  group->addButton(discreteRadio);
  group->addButton(relativeRadio);
  group->addButton(rangeRadio);

  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addWidget(discreteRadio);
  layout->addWidget(relativeRadio);
  layout->addWidget(rangeRadio);
  layout->addStretch(1);

  setPage(PageChooseType, page);
}

void StateMachineWizard::buildDefineStatesPage()
{
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("Define states"));
  page->setSubTitle(
      tr(
          "Discrete: value names. Relative: values in cycle order, drag to reorder."));

  statesList = new QListWidget(page);
  statesList->setDragDropMode(QAbstractItemView::InternalMove);

  addStateButton = new QPushButton(tr("Add"), page);
  connect(addStateButton, &QPushButton::clicked, this,
      &StateMachineWizard::onAddStateClicked);

  removeStateButton = new QPushButton(tr("Remove"), page);
  connect(removeStateButton, &QPushButton::clicked, this,
      &StateMachineWizard::onRemoveStateClicked);

  QVBoxLayout *buttonsLayout = new QVBoxLayout();
  buttonsLayout->addWidget(addStateButton);
  buttonsLayout->addWidget(removeStateButton);
  buttonsLayout->addStretch(1);

  QHBoxLayout *rowLayout = new QHBoxLayout();
  rowLayout->addWidget(statesList, 1);
  rowLayout->addLayout(buttonsLayout);

  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addLayout(rowLayout);

  setPage(PageDefineStates, page);
}

void StateMachineWizard::buildAssignActionsPage()
{
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("Assign actions"));
  page->setSubTitle(
      tr(
          "Pick a slot on the left, then edit its steps on the right. Can be skipped and filled in later."));

  actionSlotsList = new QListWidget(page);
  connect(actionSlotsList, &QListWidget::currentRowChanged, this,
      &StateMachineWizard::onActionSlotChanged);

  actionEditor = new DeviceActionEditor(page);
  actionEditor->setEnabled(false);

  QHBoxLayout *layout = new QHBoxLayout(page);
  layout->addWidget(actionSlotsList, 0);
  layout->addWidget(actionEditor, 1);

  setPage(PageAssignActions, page);
}

void StateMachineWizard::buildReviewPage()
{
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("Review and finish"));

  reviewSummaryLabel = new QLabel(page);
  reviewSummaryLabel->setWordWrap(true);

  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addWidget(reviewSummaryLabel);
  layout->addStretch(1);

  setPage(PageReview, page);
}

void StateMachineWizard::rebuildActionSlots()
{
  storeCurrentSlotEdits();

  actionSlotsList->clear();

  if (discreteRadio->isChecked()) {
    for (int i = 0; i < statesList->count(); ++i) {
      const QString slotKey = tr("Set: %1").arg(statesList->item(i)->text());
      actionSlotsList->addItem(slotKey);
      if (!actionsBySlot.contains(slotKey)) {
        actionsBySlot.insert(slotKey, DeviceAction { });
      }
    }
  } else {
    actionSlotsList->addItem(tr("Next action"));
    actionSlotsList->addItem(tr("Prev action"));
    actionSlotsList->addItem(tr("Reset action"));
    if (!actionsBySlot.contains(tr("Next action"))) {
      actionsBySlot.insert(tr("Next action"), DeviceAction { });
    }
  }

  if (actionSlotsList->count() > 0) {
    actionSlotsList->setCurrentRow(0);
  }
}

void StateMachineWizard::storeCurrentSlotEdits()
{
  if (currentSlotKey.isEmpty() || !actionEditor->isEnabled()) {
    return;
  }

  actionsBySlot.insert(currentSlotKey, actionEditor->getDeviceAction());
}

QString StateMachineWizard::reviewSummaryText() const
{
  const QString typeName =
      discreteRadio->isChecked() ? tr("Discrete") : tr("Relative");

  QStringList stateNames;
  for (int i = 0; i < statesList->count(); ++i) {
    stateNames.append(statesList->item(i)->text());
  }

  int configuredSlots = 0;
//  for (auto it = actionsBySlot.constBegin(); it != actionsBySlot.constEnd();
//      ++it) {
//    if (!it.value().empty()) {
//      ++configuredSlots;
//    }
//  }

  return tr("Type: %1\nStates: %2\nAction slots configured: %3 of %4").arg(
      typeName, stateNames.join(QStringLiteral(", "))).arg(configuredSlots).arg(
      actionSlotsList->count());
}

}
