// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QSpinBox>
#include <QStackedWidget>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include "stateMachineDetailPanel.h"
#include "discreteStateEditor.h"
#include "relativeStateEditor.h"

namespace document
{
namespace data
{
namespace item
{

namespace
{
constexpr int StackIndexEmpty = 0;
constexpr int StackIndexDiscrete = 1;
constexpr int StackIndexRelative = 2;
}

StateMachineDetailPanel::StateMachineDetailPanel(QWidget *parent) :
    QWidget(parent)
{
  buildUi();
}

void StateMachineDetailPanel::setStateMachine(const StateMachine &stateMachine)
{
  currentType =
      stateMachine.discrete.empty() ?
          StateMachineType::Relative : StateMachineType::Discrete;

  smTypeLabel->setText(
      QString::fromStdString(stateMachine.smType.get().getString()));
  delaySpin->setValue(static_cast<int>(stateMachine.delayMs.get()));

  if (currentType == StateMachineType::Discrete) {
    discreteEditor->setDiscreteActions(stateMachine.discrete);
  } else {
    relativeEditor->setRelativeActions(stateMachine.relative);
  }

  typeStack->setCurrentIndex(stackIndexForType(currentType));
}

StateMachine StateMachineDetailPanel::getStateMachine() const
{
  StateMachine stateMachine;

  stateMachine.smType.set(
      Enum<StateMachineDeviceType>(smTypeLabel->text().toStdString()));
  stateMachine.delayMs.set(static_cast<uint32_t>(delaySpin->value()));

  if (currentType == StateMachineType::Discrete) {
    stateMachine.discrete = discreteEditor->getDiscreteActions();
  } else if (currentType == StateMachineType::Relative) {
    stateMachine.relative = relativeEditor->getRelativeActions();
  }

  return stateMachine;
}

void StateMachineDetailPanel::showEmptyState()
{
  typeStack->setCurrentIndex(StackIndexEmpty);
}

void StateMachineDetailPanel::buildUi()
{
  smTypeLabel = new QLabel(this);
  smTypeLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

  delaySpin = new QSpinBox(this);
  delaySpin->setRange(0, 600000);
  delaySpin->setSuffix(tr(" ms"));

  rerunWizardButton = new QToolButton(this);
  rerunWizardButton->setText(tr("Edit with wizard..."));
  connect(rerunWizardButton, &QToolButton::clicked, this,
      &StateMachineDetailPanel::rerunWizardRequested);

  deleteButton = new QToolButton(this);
  deleteButton->setText(tr("Delete"));
  connect(deleteButton, &QToolButton::clicked, this,
      &StateMachineDetailPanel::deleteRequested);

  QFormLayout *headerForm = new QFormLayout();
  headerForm->addRow(tr("Type"), smTypeLabel);
  headerForm->addRow(tr("Delay"), delaySpin);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->addLayout(headerForm);
  headerLayout->addStretch(1);
  headerLayout->addWidget(rerunWizardButton);
  headerLayout->addWidget(deleteButton);

  emptyStatePage = new QWidget(this);
  QVBoxLayout *emptyLayout = new QVBoxLayout(emptyStatePage);
  QLabel *emptyLabel = new QLabel(
      tr("Select a state machine on the left, or add a new one."),
      emptyStatePage);
  emptyLabel->setAlignment(Qt::AlignCenter);
  emptyLabel->setEnabled(false);
  emptyLayout->addStretch(1);
  emptyLayout->addWidget(emptyLabel);
  emptyLayout->addStretch(1);

  discreteEditor = new DiscreteStateEditor(this);
  connect(discreteEditor, &DiscreteStateEditor::changed, this,
      &StateMachineDetailPanel::changed);

  relativeEditor = new RelativeStateEditor(this);
  connect(relativeEditor, &RelativeStateEditor::changed, this,
      &StateMachineDetailPanel::changed);

  typeStack = new QStackedWidget(this);
  typeStack->insertWidget(StackIndexEmpty, emptyStatePage);
  typeStack->insertWidget(StackIndexDiscrete, discreteEditor);
  typeStack->insertWidget(StackIndexRelative, relativeEditor);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addLayout(headerLayout);
  rootLayout->addWidget(typeStack, 1);

  showEmptyState();
}

int StateMachineDetailPanel::stackIndexForType(StateMachineType type) const
{
  switch (type) {
    case StateMachineType::Discrete:
      return StackIndexDiscrete;
    case StateMachineType::Relative:
      return StackIndexRelative;
    case StateMachineType::Unknown:
      return StackIndexEmpty;
  }
  return StackIndexEmpty;
}

}
}
}
