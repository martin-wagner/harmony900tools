// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QComboBox>
#include <QStackedWidget>
#include <QToolButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include "lib/icon.h"
#include "lib/qtHelpers.h"
#include "document/config.h"
#include "sequenceItemWidget.h"

using namespace std;
using namespace document::data;
using namespace document::data::item;

namespace editors
{

SequenceItemWidget::SequenceItemWidget(Context &ctx, uint32_t devicePos,
    uint32_t smPos, ParentType t, QWidget *parent) :
    QWidget(parent), config(*ctx.config()), devicePos(devicePos), smPos(smPos)
{
  createView(t);
  createConnections();
}

void SequenceItemWidget::setSequenceItem(const SequenceItem &sequenceItem)
{
  QString text;

  item = sequenceItem;

  if ((devicePos == 0xffffffff) || (smPos == 0xffffffff)) {
    return;
  }

  opcodeCombo->setCurrentText(item.opcode.get().getQString());
  cmdCombo->setCurrentText(qstr(item.cmd.get()));
  modCombo->setCurrentText(item.mod.get().getQString());
  delayBox->setValue(item.delayMs.get());
  if (item.deviceId.isIncluded() == Used::YES) {
    auto id = item.deviceId.get();
    auto *device = config.data().getDevice(id);
    if (device != nullptr) {
      text = qstr(device->label.get());
    } else {
      //fallback
      text = QString::number(id);
    }
    if (deviceListCombo->findText(text) == -1) {
      deviceListCombo->addItem(text);
    }
    deviceListCombo->setCurrentText(text);
  } //else keep empty
  valueBox->setValue(atol(item.value.get().c_str()));
  stateNameCombo->setCurrentText(item.stateName.get().getQString());
  valueCombo->setCurrentText(qstr(item.value.get()));
  forcedStateNameCombo->setCurrentText(item.stateName.get().getQString());
  forcedStateValueCombo->setCurrentText(qstr(item.value.get()));
}

SequenceItem SequenceItemWidget::getSequenceItem() const
{
  SequenceItem ret;

  auto opcode = Enum<Operation>(opcodeCombo->currentText());
  if (item.opcode.get().getValue() == opcode.getValue()) {
    ret = item; //copy trough, otherwise create new one.
  }
  ret.opcode.set(opcode);

  switch (opcode.getValue()) {
    case Operation::SendCommand:
      ret.cmd.set(cmdCombo->currentText().toStdString()).setIncluded(Used::YES);
      ret.mod.set(Enum<Modifier>(cmdCombo->currentText())).setIncluded(
          Used::YES);
      break;
    case Operation::SetValue:
      ret.stateName.set(
          Enum<StateMachineDeviceType>(stateNameCombo->currentText())).setIncluded(
          Used::YES);
      ret.value.set(valueCombo->currentText().toStdString()).setIncluded(
          Used::YES);
      break;
    case Operation::SendDelay:
      ret.delayMs.set(delayBox->value()).setIncluded(Used::YES);
      break;
    case Operation::SendFlush: {
      auto label = deviceListCombo->currentText().toStdString();
      ret.deviceId.set(atoi(label.c_str())).setIncluded(Used::YES); //import fallback is plain device id
      for (const auto &device : config.data().getDevices()) {
        if (device.label.get() == label) {
          ret.deviceId.set(device.getId()).setIncluded(Used::YES);
          break;
        }
      }
      break;
    }
    case Operation::SendNumber:
      ret.value = to_string(valueBox->value());
      break;
    case Operation::ForceValue:
      ret.stateName.set(
          Enum<StateMachineDeviceType>(forcedStateNameCombo->currentText())).setIncluded(
          Used::YES);
      ret.value.set(forcedStateValueCombo->currentText().toStdString()).setIncluded(
          Used::YES);
      break;
    default:
      break;
  }
  return ret;
}

void SequenceItemWidget::setDragHandleVisible(bool visible)
{
  dragHandle->setVisible(visible);
}

void SequenceItemWidget::onOperationChanged(const QString &text)
{
  auto opcode = Enum<Operation>(text).getValue();
  switch (opcode) { //follow buildParameterPages()
    case Operation::SendCommand:
      parameterStack->setCurrentIndex(0);
      break;
    case Operation::SetValue:
      parameterStack->setCurrentIndex(4);
      onStateNameComboChanged(stateNameCombo->currentText());
      break;
    case Operation::SendDelay:
      parameterStack->setCurrentIndex(1);
      break;
    case Operation::SendFlush:
      parameterStack->setCurrentIndex(2);
      break;
    case Operation::SendNumber:
      parameterStack->setCurrentIndex(3);
      break;
    case Operation::ForceValue:
      parameterStack->setCurrentIndex(5);
      onForcedStateNameComboChanged(forcedStateNameCombo->currentText());
      break;
    default:
      break;
  }

  if (item.opcode.get().getValue() != opcode) {
    emit changed();
  }
}

void SequenceItemWidget::onStateNameComboChanged(const QString &text)
{
  valueCombo->clear();

  auto &machines = getMachines();
  for (const auto &machine : machines) {
    if (text == machine.smType.get().getQString()) {
      if (!machine.discrete.empty()) {
        valueCombo->addItems(lib::toQStringList(machine.discrete.states));
      } else if (!machine.relative.empty()) {
        valueCombo->addItems(lib::toQStringList(machine.relative.states));
      }
      return;
    }
  }
}

void SequenceItemWidget::onForcedStateNameComboChanged(const QString &text)
{
  forcedStateValueCombo->clear();

  auto &machines = getMachines();
  for (const auto &machine : machines) {
    if (text == machine.smType.get().getQString()) {
      if (!machine.discrete.empty()) {
        forcedStateValueCombo->addItems(
            lib::toQStringList(machine.discrete.states));
      } else if (!machine.relative.empty()) {
        forcedStateValueCombo->addItems(
            lib::toQStringList(machine.relative.states));
      }
      return;
    }
  }
}

const Device& SequenceItemWidget::getDevice() const
{
  return config.data().getDevices().at(devicePos);
}

const vector<StateMachine>& SequenceItemWidget::getMachines() const
{
  return getDevice().getStateMachines();
}

const StateMachine& SequenceItemWidget::getMachine() const
{
  return getMachines().at(smPos);
}

void SequenceItemWidget::createView(ParentType t)
{
  dragHandle = new QLabel(QStringLiteral("::"), this);
  dragHandle->setToolTip(tr("Drag to reorder"));

  targetCombo = new QComboBox(this);
  targetCombo->addItem("Device");
  targetCombo->addItem("Activity");
  if (t == ParentType::DEVICE) {
    targetCombo->setCurrentText("Device");
  } else {
    targetCombo->setCurrentText("Activity");
  }
  targetCombo->setVisible(false);

  opcodeCombo = new QComboBox(this);
  opcodeCombo->addItem(Enum<Operation>::toQString(Operation::SendCommand));
  opcodeCombo->addItem(Enum<Operation>::toQString(Operation::SendDelay));
  opcodeCombo->addItem(Enum<Operation>::toQString(Operation::SendFlush));
  opcodeCombo->addItem(Enum<Operation>::toQString(Operation::SendNumber));
  opcodeCombo->addItem(Enum<Operation>::toQString(Operation::SetValue));
  opcodeCombo->addItem(Enum<Operation>::toQString(Operation::ForceValue));

  parameterStack = new QStackedWidget(this);
  buildParameterPages();

  removeButton = new QToolButton(this);
  removeButton->setIcon(lib::getDeleteIcon());
  removeButton->setToolTip(tr("Remove this step"));
  removeButton->setAutoRaise(true);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->addWidget(dragHandle);
  headerLayout->addWidget(targetCombo);
  headerLayout->addWidget(opcodeCombo);
  headerLayout->addStretch(1);
  headerLayout->addWidget(removeButton);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addLayout(headerLayout);
  rootLayout->addWidget(parameterStack);

  opcodeCombo->setCurrentIndex(0);
  parameterStack->setCurrentIndex(0);
}

void SequenceItemWidget::createConnections()
{
  connect(opcodeCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::onOperationChanged);
  connect(removeButton, &QToolButton::clicked, this,
      &SequenceItemWidget::removeRequested);

  connect(cmdCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::changed);
  connect(modCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::changed);

  connect(delayBox, &QSpinBox::valueChanged, this,
      &SequenceItemWidget::changed);

  connect(deviceListCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::changed);

  connect(valueBox, &QSpinBox::valueChanged, this,
      &SequenceItemWidget::changed);

  connect(stateNameCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::changed);
  connect(stateNameCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::onStateNameComboChanged);
  connect(valueCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::changed);

  connect(forcedStateNameCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::changed);
  connect(forcedStateNameCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::onForcedStateNameComboChanged);
  connect(forcedStateValueCombo, &QComboBox::currentTextChanged, this,
      &SequenceItemWidget::changed);
}

void SequenceItemWidget::buildParameterPages()
{
  parameterStack->addWidget(buildSendCommandPage());
  parameterStack->addWidget(buildSendDelayPage());
  parameterStack->addWidget(buildSendFlushPage());
  parameterStack->addWidget(buildSendNumberPage());
  parameterStack->addWidget(buildSetValuePage());
  parameterStack->addWidget(buildForceValuePage());
}

QWidget* SequenceItemWidget::buildSendCommandPage()
{
  QWidget *page = new QWidget(this);

  auto commandList = lib::toQStringList(
      getDevice().getIrCommands().getAvailableCommands());

  cmdCombo = new QComboBox(page);
  cmdCombo->addItems(commandList);

  modCombo = new QComboBox(page);
  modCombo->addItem(Enum<Modifier>::toQString(Modifier::Press));
  modCombo->addItem(Enum<Modifier>::toQString(Modifier::Hold));
  modCombo->setEnabled(false); //never seen anything other than "press"

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Command"), cmdCombo);
  formLayout->addRow(tr("Modifier"), modCombo);

  return page;
}

QWidget* SequenceItemWidget::buildSendDelayPage()
{
  QWidget *page = new QWidget(this);

  delayBox = new QSpinBox(page);
  delayBox->setRange(0, 100000);
  delayBox->setSingleStep(100);
  delayBox->setSuffix(tr(" ms"));

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Delay "), delayBox);

  return page;
}

QWidget* SequenceItemWidget::buildSendFlushPage()
{
  QWidget *page = new QWidget(this);

  auto deviceList = lib::toQStringList(config.data().getDeviceLabels());
  deviceList.removeAll(qstr(getDevice().label.get())); //remove own device

  deviceListCombo = new QComboBox(page);
  deviceListCombo->addItems(deviceList);

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("to this device "), deviceListCombo);

  return page;
}

QWidget* SequenceItemWidget::buildSendNumberPage()
{
  QWidget *page = new QWidget(this);

  valueBox = new QSpinBox(page);
  valueBox->setToolTip(tr("max three digits"));
  valueBox->setRange(0, 999); //fixme check number of supported digits. assumption: 3

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Number"), valueBox);

  return page;
}

QWidget* SequenceItemWidget::buildSetValuePage()
{
  QStringList smList;

  QWidget *page = new QWidget(this);

  auto &device = getDevice();
  for (const auto &state : device.getStateMachines()) {
    smList.push_back(state.smType.get().getQString());
  }
  smList.removeAll(getMachine().smType.get().getQString()); //remove own machine

  stateNameCombo = new QComboBox(page);
  stateNameCombo->addItems(smList);
  valueCombo = new QComboBox(page);
  //values added based on base statemachine

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Control"), stateNameCombo);
  formLayout->addRow(tr("State value"), valueCombo);

  return page;
}

QWidget* SequenceItemWidget::buildForceValuePage()
{
  QStringList smList;

  QWidget *page = new QWidget(this);
  page->setToolTip(
      tr("The same as \"SetValue\". No idea where the difference is."));

  auto &device = getDevice();
  for (const auto &state : device.getStateMachines()) {
    smList.push_back(state.smType.get().getQString());
  }
  smList.removeAll(getMachine().smType.get().getQString()); //remove own machine

  forcedStateNameCombo = new QComboBox(page);
  forcedStateNameCombo->addItems(smList);
  forcedStateValueCombo = new QComboBox(page);
  //values added based on base statemachine

  QFormLayout *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Control"), forcedStateNameCombo);
  formLayout->addRow(tr("State value"), forcedStateValueCombo);

  return page;
}

}
