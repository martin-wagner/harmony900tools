// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QSpinBox>
#include <QStackedWidget>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>

#include "lib/icon.h"
#include "document/config.h"
#include "stateMachineDetailPanel.h"
#include "discreteStateEditor.h"
#include "relativeStateEditor.h"
#include "deviceActionEditor.h"
#include "stateMachineWizard.h"

using namespace std;
using namespace document::data::item;

namespace editors
{

constexpr int StackIndexEmpty = 0;
constexpr int StackIndexDiscrete = 1;
constexpr int StackIndexRelative = 2;

StateMachineDetailPanel::StateMachineDetailPanel(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx), config(*ctx.config())
{
  createView(ctx);
  createConnections();
}

void StateMachineDetailPanel::setStateMachine(uint32_t devicePos,
    uint32_t smPos)
{
  this->devicePos = devicePos;
  this->smPos = smPos;
  if (!machineIsValid()) {
    showEmptyState();
    return;
  }
  auto &m = getMachine();

  if (!m.discrete.empty()) {
    currentType = StateMachineType::Discrete;
  } else if (!m.relative.empty()) {
    currentType = StateMachineType::Relative;
  } else {
    currentType = StateMachineType::Unknown;
  }
  smTypeLabel->setEnabled(true);
  delaySpinBox->setEnabled(true);
  startGroup->setEnabled(true);
  finishGroup->setEnabled(true);

  switch (currentType) {
    case StateMachineType::Discrete:
      discreteEditor->setDiscreteActions(devicePos, smPos);
      break;
    case StateMachineType::Relative:
      relativeEditor->setRelativeActions(devicePos, smPos);
      break;
    default:
      showEmptyState();
      return;
  }

  typeStack->setCurrentIndex(stackIndexForType(currentType));

  updateData();
}

void StateMachineDetailPanel::updateData()
{
  if (!machineIsValid()) {
    showEmptyState();
    return;
  }
  auto &m = getMachine();

  smTypeLabel->setText(m.smType.get().getQString());
  delaySpinBox->setValue(static_cast<int>(m.delayMs.get()));
  if (m.startAction.has_value()) {
    startActionButton->setText(
        tr("%n step(s)", "", static_cast<int>(m.startAction->sequence.size())));
  } else {
    startActionButton->setText(tr("Add"));
  }
  if (m.finishAction.has_value()) {
    finishActionButton->setText(
        tr("%n step(s)", "",
            static_cast<int>(m.finishAction->sequence.size())));
  } else {
    finishActionButton->setText(tr("Add"));
  }

  switch (currentType) {
    case StateMachineType::Discrete:
      discreteEditor->updateData();
      break;
    case StateMachineType::Relative:
      relativeEditor->updateData();
      break;
    default:
      break;
  }
}

void StateMachineDetailPanel::showEmptyState()
{
  currentType = StateMachineType::Unknown;
  smTypeLabel->setText("--<===>--");
  smTypeLabel->setEnabled(false);
  delaySpinBox->setValue(0);
  delaySpinBox->setEnabled(false);
  startGroup->setEnabled(false);
  startActionButton->setText(tr("Add"));
  finishGroup->setEnabled(false);
  finishActionButton->setText(tr("Add"));

  typeStack->setCurrentIndex(StackIndexEmpty);
}

void StateMachineDetailPanel::onEditingDelayFinished()
{
  auto res = config.modify().setDeviceStatemachineDelay(delaySpinBox->value(),
      devicePos, smPos);
  if (res != true) {
    updateData(); //revert
  }
}

void StateMachineDetailPanel::onRunWizardClicked()
{
  if (!machineIsValid()) {
    return;
  }
  auto machine = getMachine();

  auto wizard = StateMachineWizard(ctx, devicePos, this);
  wizard.setStateMachine(machine);
  if (wizard.exec() == QDialog::Accepted) {
    auto newMachine = wizard.getStateMachine();
    if (newMachine.relative.empty() && newMachine.discrete.empty()) {
      QMessageBox msgBox(QMessageBox::Warning, tr("No magic"),
          tr("Oops, something went wrong. Your Control is empty."),
          QMessageBox::Ok);
      msgBox.exec();
      return;
    }

    config.beginMacro("Change device control");
    config.modify().setDeviceStatemachine(newMachine, devicePos, smPos);
    config.endMacro();
  }
  setStateMachine(devicePos, smPos); //trigger update "this" with new data (or old data if setting failed)
}

void StateMachineDetailPanel::onEditStartActionClicked()
{
  auto res = true;
  auto &m = getMachine();
  auto a = m.startAction.value_or(DeviceAction());

  auto data = DeviceActionEditor::openEditor(ctx, devicePos, smPos, a,
      document::data::ActionType::StartAction, tr("Actions for starting"),
      this);
  if (!data.has_value()) {
    return;
  }

  config.beginMacro("Edit Start Action");

  if (!m.startAction.has_value()) {
    res &= config.modify().addDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Start);
  }
  res &= config.modify().setDeviceSmAction(data.value(), devicePos, smPos,
      StateMachineAction::Start, 0);

  config.endMacro();

  if (res != true) {
    updateData(); //revert
  }
}

void StateMachineDetailPanel::onEditFinishActionClicked()
{
  bool res = true;
  auto &m = getMachine();
  auto a = m.finishAction.value_or(DeviceAction());

  auto data = DeviceActionEditor::openEditor(ctx, devicePos, smPos, a,
      document::data::ActionType::FinishAction, tr("Actions for finishing"),
      this);
  if (!data.has_value()) {
    return;
  }

  config.beginMacro("Edit Finish Action");

  if (!m.finishAction.has_value()) {
    res &= config.modify().addDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Finish);
  }
  res &= config.modify().setDeviceSmAction(data.value(), devicePos, smPos,
      StateMachineAction::Finish, 0);

  config.endMacro();
}

void StateMachineDetailPanel::onClearStartActionClicked()
{
  if (getMachine().startAction.has_value()) {
    config.beginMacro("Remove Start Action");
    auto res = config.modify().removeDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Start);
    config.endMacro();

    if (res != true) {
      updateData(); //revert
    }
  }
}

void StateMachineDetailPanel::onClearFinishActionClicked()
{
  if (getMachine().finishAction.has_value()) {
    config.beginMacro("Remove Finish Action");
    auto res = config.modify().removeDeviceSmActionCommand(devicePos, smPos,
        StateMachineAction::Finish);
    config.endMacro();

    if (res != true) {
      updateData(); //revert
    }
  }
}

const StateMachine& StateMachineDetailPanel::getMachine() const
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

  wizardButton = new QToolButton(this);
  wizardButton->setText(tr("Create with wizard..."));

  QFormLayout *headerForm = new QFormLayout();
  headerForm->addRow(tr("Type"), smTypeLabel);
  headerForm->addRow(tr("Delay"), delaySpinBox);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->addLayout(headerForm);
  headerLayout->addStretch(1);
  headerLayout->addWidget(wizardButton);

  startActionButton = new QPushButton(lib::getEditIcon(), tr("Add"), this);
  startActionClearButton = new QPushButton(lib::getDeleteIcon(), "", this);
  startGroup = new QGroupBox(tr("Start action (optional)"), this);
  startGroup->setToolTip("This is done first (e.g. open selection menu)");
  QHBoxLayout *startLayout = new QHBoxLayout(startGroup);
  startLayout->addWidget(startActionButton, 1);
  startLayout->addWidget(startActionClearButton, 0);
  startGroup->setEnabled(false);

  finishActionButton = new QPushButton(lib::getEditIcon(), tr("Add"), this);
  finishActionClearButton = new QPushButton(lib::getDeleteIcon(), "", this);
  finishGroup = new QGroupBox(tr("Finish action (optional)"), this);
  finishGroup->setToolTip("This is done at the end (e.g. confirm selection)");
  QHBoxLayout *finishLayout = new QHBoxLayout(finishGroup);
  finishLayout->addWidget(finishActionButton, 1);
  finishLayout->addWidget(finishActionClearButton, 0);
  finishGroup->setEnabled(false);

  QHBoxLayout *actionLayout = new QHBoxLayout();
  actionLayout->addWidget(startGroup, 1);
  actionLayout->addWidget(finishGroup, 1);

  emptyStatePage = new QWidget(this);
  QVBoxLayout *emptyLayout = new QVBoxLayout(emptyStatePage);
  QLabel *emptyLabel = new QLabel(
      tr("Select a state machine on the left, or add\n"
          "a new one using the \"wizard\" button."), emptyStatePage);
  emptyLabel->setAlignment(Qt::AlignCenter);
  emptyLabel->setEnabled(false);
  emptyLayout->addStretch(1);
  emptyLayout->addWidget(emptyLabel);
  emptyLayout->addStretch(1);

  discreteEditor = new DiscreteStateEditor(ctx, this);

  relativeEditor = new RelativeStateEditor(ctx, this);

  typeStack = new QStackedWidget(this);
  typeStack->insertWidget(StackIndexEmpty, emptyStatePage);
  typeStack->insertWidget(StackIndexDiscrete, discreteEditor);
  typeStack->insertWidget(StackIndexRelative, relativeEditor);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addLayout(headerLayout);
  rootLayout->addLayout(actionLayout);
  rootLayout->addWidget(typeStack, 1);

  showEmptyState();
}

void StateMachineDetailPanel::createConnections()
{
  connect(startActionButton, &QPushButton::clicked, this,
      &StateMachineDetailPanel::onEditStartActionClicked);
  connect(startActionClearButton, &QPushButton::clicked, this,
      &StateMachineDetailPanel::onClearStartActionClicked);
  connect(finishActionButton, &QPushButton::clicked, this,
      &StateMachineDetailPanel::onEditFinishActionClicked);
  connect(finishActionClearButton, &QPushButton::clicked, this,
      &StateMachineDetailPanel::onClearFinishActionClicked);
  connect(wizardButton, &QToolButton::clicked, this,
      &StateMachineDetailPanel::onRunWizardClicked);
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
