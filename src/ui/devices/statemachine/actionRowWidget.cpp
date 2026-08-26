// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QComboBox>
#include <QStackedWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include "lib/qtHelpers.h"
#include "actionRowWidget.h"

using namespace document::data::item;

namespace editors
{

ActionRowWidget::ActionRowWidget(QWidget *parent) :
    QWidget(parent)
{
  buildUi();
}

void ActionRowWidget::setAction(const DeviceAction &action)
{
//  if (action.target == Action::Target::Activity) {
//    targetCombo->setCurrentIndex(1);
//  } else {
//    targetCombo->setCurrentIndex(0);
//  }
//
//  const std::string operationName = action.operation.getString();
//  const int operationIndex = operationCombo->findData(
//      qstr(operationName));
//  if (operationIndex >= 0) {
//    operationCombo->setCurrentIndex(operationIndex);
//  }
//
//  for (const auto &parameter : action.parameters) {
//    const QString name = qstr(parameter.first);
//    const QString value = qstr(parameter.second);
//
//    if (name == "Command") {
//      commandCombo->setCurrentText(value);
//    } else if (name == "Modifier") {
//      modifierCombo->setCurrentText(value);
//    } else if (name == "Delay") {
//      delayEdit->setText(value);
//    } else if (name == "Number") {
//      numberEdit->setText(value);
//    } else if (name == "State") {
//      stateNameEdit->setText(value);
//    } else if (name == "Value") {
//      valueEdit->setText(value);
//    }
//  }
}

DeviceAction ActionRowWidget::getAction() const
{
  DeviceAction action;
//
//  if (targetCombo->currentIndex() == 1) {
//    action.target = Action::Target::Activity;
//  } else {
//    action.target = Action::Target::Device;
//  }
//
//  const QString operationName = operationCombo->currentData().toString();
//  action.operation = Enum<Operation>(operationName.toStdString());
//
//  if (operationName == "SendCommand") {
//    action.parameters.emplace_back("Command",
//        commandCombo->currentText().toStdString());
//    action.parameters.emplace_back("Modifier",
//        modifierCombo->currentText().toStdString());
//  } else if (operationName == "SendDelay") {
//    action.parameters.emplace_back("Delay", delayEdit->text().toStdString());
//  } else if (operationName == "SendFlush") {
//    //no parameters
//  } else if (operationName == "SendNumber") {
//    action.parameters.emplace_back("Number", numberEdit->text().toStdString());
//  } else if (operationName == "SetValue") {
//    action.parameters.emplace_back("State",
//        stateNameEdit->text().toStdString());
//    action.parameters.emplace_back("Value", valueEdit->text().toStdString());
//  } else if (operationName == "ForceValue") {
//    action.parameters.emplace_back("State",
//        forceStateNameEdit->text().toStdString());
//    action.parameters.emplace_back("Value",
//        forceValueEdit->text().toStdString());
//  }
//
//  return action;
}

void ActionRowWidget::setDragHandleVisible(bool visible)
{
  dragHandle->setVisible(visible);
}

void ActionRowWidget::onOperationChanged(int index)
{
  parameterStack->setCurrentIndex(index);
  emit changed();
}

void ActionRowWidget::buildUi()
{
  dragHandle = new QLabel(QStringLiteral("::"), this);
  dragHandle->setToolTip(tr("Drag to reorder"));

  targetCombo = new QComboBox(this);
  targetCombo->addItem(tr("Device"), QStringLiteral("Device"));
  targetCombo->addItem(tr("Activity"), QStringLiteral("Activity"));
  //Activity targets are not exposed in this UI yet -- kept alive (not
  //deleted) so state can round-trip once activities are wired up, but
  //never shown or reachable by the user.
  targetCombo->setVisible(false);

  operationCombo = new QComboBox(this);
  operationCombo->addItem(tr("Send command"), QStringLiteral("SendCommand"));
  operationCombo->addItem(tr("Send delay"), QStringLiteral("SendDelay"));
  operationCombo->addItem(tr("Send flush"), QStringLiteral("SendFlush"));
  operationCombo->addItem(tr("Send number"), QStringLiteral("SendNumber"));
  operationCombo->addItem(tr("Set value"), QStringLiteral("SetValue"));
  operationCombo->addItem(tr("Force value"), QStringLiteral("ForceValue"));

  parameterStack = new QStackedWidget(this);
  buildParameterPages();

  removeButton = new QToolButton(this);
  removeButton->setText(QStringLiteral("\u00D7")); //multiplication sign, used as a close glyph
  removeButton->setToolTip(tr("Remove this step"));
  removeButton->setAutoRaise(true);

  connect(operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
      this, &ActionRowWidget::onOperationChanged);
  connect(removeButton, &QToolButton::clicked, this,
      &ActionRowWidget::removeRequested);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->addWidget(dragHandle);
  headerLayout->addWidget(targetCombo);
  headerLayout->addWidget(operationCombo);
  headerLayout->addStretch(1);
  headerLayout->addWidget(removeButton);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addLayout(headerLayout);
  rootLayout->addWidget(parameterStack);

  operationCombo->setCurrentIndex(0);
  parameterStack->setCurrentIndex(0);
}

void ActionRowWidget::buildParameterPages()
{
  parameterStack->addWidget(buildSendCommandPage());
  parameterStack->addWidget(buildSendDelayPage());
  parameterStack->addWidget(buildSendFlushPage());
  parameterStack->addWidget(buildSendNumberPage());
  parameterStack->addWidget(buildSetValuePage());
  parameterStack->addWidget(buildForceValuePage());
}

QWidget* ActionRowWidget::buildSendCommandPage()
{
  QWidget *page = new QWidget(this);

  commandCombo = new QComboBox(page);
  commandCombo->setEditable(true); //command list comes from the device's Commands, filled in by caller

  modifierCombo = new QComboBox(page);
  modifierCombo->addItem(tr("Press"), QStringLiteral("Press"));
  modifierCombo->addItem(tr("Hold"), QStringLiteral("Hold"));
  modifierCombo->addItem(tr("None"), QStringLiteral("None"));

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Command"), commandCombo);
  formLayout->addRow(tr("Modifier"), modifierCombo);

  return page;
}

QWidget* ActionRowWidget::buildSendDelayPage()
{
  QWidget *page = new QWidget(this);

  delayEdit = new QLineEdit(page);
  delayEdit->setPlaceholderText(tr("milliseconds"));

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Delay (ms)"), delayEdit);

  return page;
}

QWidget* ActionRowWidget::buildSendFlushPage()
{
  QWidget *page = new QWidget(this);

  QVBoxLayout *layout = new QVBoxLayout(page);
  QLabel *note = new QLabel(tr("No parameters for this operation."), page);
  note->setEnabled(false);
  layout->addWidget(note);

  return page;
}

QWidget* ActionRowWidget::buildSendNumberPage()
{
  QWidget *page = new QWidget(this);

  numberEdit = new QLineEdit(page);

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Number"), numberEdit);

  return page;
}

QWidget* ActionRowWidget::buildSetValuePage()
{
  QWidget *page = new QWidget(this);

  stateNameEdit = new QLineEdit(page);
  valueEdit = new QLineEdit(page);

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("State"), stateNameEdit);
  formLayout->addRow(tr("Value"), valueEdit);

  return page;
}

QWidget* ActionRowWidget::buildForceValuePage()
{
  //ForceValue shares its parameter shape with SetValue (State + Value),
  //but gets its own controls so the two pages can diverge later without
  //entangling SetValue's edits with ForceValue's.
  QWidget *page = new QWidget(this);

  forceStateNameEdit = new QLineEdit(page);
  forceValueEdit = new QLineEdit(page);

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("State name"), forceStateNameEdit);
  formLayout->addRow(tr("Value"), forceValueEdit);

  return page;
}

}
