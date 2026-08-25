// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QSpinBox>
#include <QStackedWidget>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include "document/config.h"
#include "stateMachineDetailPanel.h"
#include "discreteStateEditor.h"
#include "relativeStateEditor.h"

using namespace document::data;
using namespace document::data::item;

namespace editors
{

constexpr int StackIndexEmpty = 0;
constexpr int StackIndexDiscrete = 1;
constexpr int StackIndexRelative = 2;

StateMachineDetailPanel::StateMachineDetailPanel(Context &ctx, QWidget *parent) :
    QWidget(parent), config(*ctx.config())
{
  createView(ctx);
  createConnections();
}

void StateMachineDetailPanel::setStateMachine(uint32_t devicePos,
    uint32_t smPos)
{
  this->devicePos = devicePos;
  this->smPos = smPos;

  auto &m = getMachine();

  if (!m.discrete.empty()) {
    currentType = StateMachineType::Discrete;
  } else if (!m.relative.empty()) {
    currentType = StateMachineType::Relative;
  } else {
    currentType = StateMachineType::Unknown;
  }
  smTypeLabel->setText(m.smType.get().getQString());
  delaySpinBox->setValue(static_cast<int>(m.delayMs.get()));
  smTypeLabel->setEnabled(true);
  delaySpinBox->setEnabled(true);

  switch (currentType) {
    case StateMachineType::Discrete:
      discreteEditor->setDiscreteActions(m.discrete); //todo no pointers
      break;
    case StateMachineType::Relative:
      relativeEditor->setRelativeActions(m.relative); //todo no pointers
      break;
    default:
      showEmptyState();
      break;
  }

  typeStack->setCurrentIndex(stackIndexForType(currentType));
}

void StateMachineDetailPanel::updateData()
{
  auto &m = getMachine();

  smTypeLabel->setText(m.smType.get().getQString());
  delaySpinBox->setValue(static_cast<int>(m.delayMs.get()));

  //todo pass on to sub pages
}

void StateMachineDetailPanel::showEmptyState()
{
  devicePos = -1;
  smPos = -1;
  currentType = StateMachineType::Unknown;
  smTypeLabel->setText("--<===>--");
  smTypeLabel->setEnabled(false);
  delaySpinBox->setValue(0);
  delaySpinBox->setEnabled(false);

  typeStack->setCurrentIndex(StackIndexEmpty);
}

void StateMachineDetailPanel::onEditingDelayFinished()
{
  config.modify().setDeviceStatemachineDelay(delaySpinBox->value(), devicePos,
      smPos);
}

const document::data::item::StateMachine& StateMachineDetailPanel::getMachine() const
{
  return config.data().getDevices().at(devicePos).getStateMachines().at(smPos);
}

void StateMachineDetailPanel::createView(Context &ctx)
{
  smTypeLabel = new QLabel(this);
  smTypeLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

  delaySpinBox = new QSpinBox(this);
  delaySpinBox->setRange(0, 120000);
  delaySpinBox->setSingleStep(100);
  delaySpinBox->setToolTip(tr("Wait for this amount of time to execute the\n"
      "control command (e.g. Power On)"));
  delaySpinBox->setSuffix(tr(" ms"));

  rerunWizardButton = new QToolButton(this);
  rerunWizardButton->setText(tr("Edit with wizard..."));

  QFormLayout *headerForm = new QFormLayout();
  headerForm->addRow(tr("Type"), smTypeLabel);
  headerForm->addRow(tr("Delay"), delaySpinBox);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->addLayout(headerForm);
  headerLayout->addStretch(1);
  headerLayout->addWidget(rerunWizardButton);

  emptyStatePage = new QWidget(this);
  QVBoxLayout *emptyLayout = new QVBoxLayout(emptyStatePage);
  QLabel *emptyLabel = new QLabel(
      tr("Select a state machine on the left, or add\n"
          "a new one using the \"add\" button."),
      emptyStatePage);
  emptyLabel->setAlignment(Qt::AlignCenter);
  emptyLabel->setEnabled(false);
  emptyLayout->addStretch(1);
  emptyLayout->addWidget(emptyLabel);
  emptyLayout->addStretch(1);

  discreteEditor = new DiscreteStateEditor(this);

  relativeEditor = new RelativeStateEditor(this);

  typeStack = new QStackedWidget(this);
  typeStack->insertWidget(StackIndexEmpty, emptyStatePage);
  typeStack->insertWidget(StackIndexDiscrete, discreteEditor);
  typeStack->insertWidget(StackIndexRelative, relativeEditor);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addLayout(headerLayout);
  rootLayout->addWidget(typeStack, 1);

  showEmptyState();
}

void StateMachineDetailPanel::createConnections()
{
  connect(rerunWizardButton, &QToolButton::clicked, this,
      &StateMachineDetailPanel::rerunWizardRequested);
  connect(delaySpinBox, &QSpinBox::editingFinished, this,
      &StateMachineDetailPanel::onEditingDelayFinished);
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
