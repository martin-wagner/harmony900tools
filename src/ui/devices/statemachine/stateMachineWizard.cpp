// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QWizardPage>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QListWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

#include "lib/keywordMatcher.h"
#include "lib/qtHelpers.h"
#include "document/config.h"
#include "stateMachineWizard.h"
#include "deviceActionEditor.h"

using namespace std;
using namespace document::data;
using namespace document::data::item;

namespace editors
{

StateMachineWizard::StateMachineWizard(Context &ctx, uint32_t devicePos,
    QWidget *parent) :
    QWizard(parent), device(ctx.config()->data().getDevices().at(devicePos))
{
  availableCommands = lib::toQStringList(
      device.getIrCommands().getAvailableCommands());

  buildChooseTypePage();
  buildChooseFunctionPage();
  buildDefineInputStatesPage();
  buildAssignCommandsPage();
  buildSetupCommandsPage();
  buildReviewPage();

  connect(this, &QWizard::currentIdChanged, this,
      &StateMachineWizard::onPageChanged);

  QPixmap logo(":/res/wizard.jpeg");
  logo = logo.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  setPixmap(QWizard::LogoPixmap, logo);
  setStartId(PageChooseType);
  setWizardStyle(ModernStyle);
  setWindowTitle(tr("Setup device control"));
  resize(400, 600);
}

void StateMachineWizard::setStateMachine(const StateMachine &stateMachine)
{
  if (!stateMachine.discrete.empty()) {
    statesList->clear();
    discreteRadio->setChecked(true);
    delaySpinBox->setValue(stateMachine.delayMs.get());

    for (std::size_t i = 0; i < stateMachine.discrete.states.size(); ++i) {
      const QString stateName = qstr(stateMachine.discrete.states[i]);
      QListWidgetItem *item = new QListWidgetItem(stateName, statesList);
      item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
  } else if (!stateMachine.relative.empty()) {
    statesList->clear();
    relativeRadio->setChecked(true);
    delaySpinBox->setValue(stateMachine.delayMs.get());

    for (const std::string &state : stateMachine.relative.states) {
      QListWidgetItem *item = new QListWidgetItem(qstr(state), statesList);
      item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
  }
  startType = stateMachine.smType.get();
  switch (startType.getValue()) {
    case StateMachineDeviceType::Power:
      powerRadio->setChecked(true);
      break;
    case StateMachineDeviceType::Input:
      inputRadio->setChecked(true);
      break;
    default:
      elseRadio->setChecked(true);
      break;
  }
  if (!unusedDeviceTypes.contains(startType.getQString())) {
    unusedDeviceTypes.push_back(startType.getQString());
  }
}

StateMachine StateMachineWizard::getStateMachine() const
{
  int i;
  StateMachine sm;

  //common
  if (powerRadio->isChecked()) {
    sm.smType.set(Enum(StateMachineDeviceType::Power));
  } else if (inputRadio->isChecked()) {
    sm.smType.set(Enum(StateMachineDeviceType::Input));
  } else {
    sm.smType.set(startType);
  }
  sm.delayMs.set(delaySpinBox->value());

  //type dependend
  if (discreteRadio->isChecked()) {
    if (elseRadio->isChecked()) {
      //append empty pair
      sm.discrete.states.push_back(tr("Start here!").toStdString());
      sm.discrete.enterStateAction.push_back(DeviceAction());
    } else {
      for (int row = 0; row < commandsTable->rowCount(); row++) {
        SequenceItem s;
        DeviceAction d;
        auto *stateItem = commandsTable->item(row, 0);
        auto state = stateItem->text();
        auto *comboBox = qobject_cast<QComboBox*>(
            commandsTable->cellWidget(row, 1));
        s.opcode.set(Enum(Operation::SendCommand));
        s.cmd.set(comboBox->currentText().toStdString()).setIncluded(Used::YES);
        s.mod.set(Enum(Modifier::Press)).setIncluded(Used::YES);
        d.actionType.set(Enum(ActionType::SetAction));
        d.sequence.push_back(s);
        //append state/action pair
        sm.discrete.states.push_back(state.toStdString());
        sm.discrete.enterStateAction.push_back(d);
      }
    }
  } else if (relativeRadio->isChecked()) {
    if (elseRadio->isChecked()) {
      //append empty state, no actions
      sm.relative.states.push_back(tr("Edit me!").toStdString());
    } else {
      //states
      for (i = 0; i < statesList->count(); i++) {
        const QString stateName = statesList->item(i)->text();
        sm.relative.states.push_back(stateName.toStdString());
      }
      //actions
      if (startCommandEnabled->isChecked()) {
        SequenceItem s;
        s.opcode.set(Enum(Operation::SendCommand));
        s.cmd.set(startCommandCombo->currentText().toStdString()).setIncluded(
            Used::YES);
        s.mod.set(Enum(Modifier::Press)).setIncluded(Used::YES);
        DeviceAction d;
        d.actionType.set(Enum(ActionType::StartAction));
        d.sequence.push_back(s);
        sm.startAction = d;
      }
      if (!nextStateCombo->currentText().isEmpty()) {
        SequenceItem s;
        s.opcode.set(Enum(Operation::SendCommand));
        s.cmd.set(nextStateCombo->currentText().toStdString()).setIncluded(
            Used::YES);
        s.mod.set(Enum(Modifier::Press)).setIncluded(Used::YES);
        DeviceAction d;
        d.actionType.set(Enum(ActionType::NextAction));
        d.sequence.push_back(s);
        sm.relative.nextStateAction = d;
      } else {
        return StateMachine();
      }
      if (previousStateEnabled->isChecked()) {
        SequenceItem s;
        s.opcode.set(Enum(Operation::SendCommand));
        s.cmd.set(previousStateCombo->currentText().toStdString()).setIncluded(
            Used::YES);
        s.mod.set(Enum(Modifier::Press)).setIncluded(Used::YES);
        DeviceAction d;
        d.actionType.set(Enum(ActionType::PrevAction));
        d.sequence.push_back(s);
        sm.relative.prevStateAction = d;
      }
      if (finishCommandEnabled->isChecked()) {
        SequenceItem s;
        s.opcode.set(Enum(Operation::SendCommand));
        s.cmd.set(finishCommandCombo->currentText().toStdString()).setIncluded(
            Used::YES);
        s.mod.set(Enum(Modifier::Press)).setIncluded(Used::YES);
        DeviceAction d;
        d.actionType.set(Enum(ActionType::FinishAction));
        d.sequence.push_back(s);
        sm.finishAction = d;
      }
    }
  }

  return sm;
}

void StateMachineWizard::onAddStateClicked()
{
  int i;
  QStringList states;
  bool ok = false;

  auto stateName = QInputDialog::getText(this, tr("Add state"),
      tr("State value"), QLineEdit::Normal, tr("NewValue"), &ok);

  if (!ok || stateName.isEmpty()) {
    return;
  }

  for (i = 0; i < statesList->count(); i++) {
    const QString stateName = statesList->item(i)->text();
    states.push_back(stateName);
  }
  stateName = lib::makeStringUnique(states, stateName);
  QListWidgetItem *item = new QListWidgetItem(stateName, statesList);
  item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void StateMachineWizard::onRemoveStateClicked()
{
  QListWidgetItem *item = statesList->currentItem();
  if (item == nullptr) {
    return;
  }

  delete statesList->takeItem(statesList->row(item));
}

void StateMachineWizard::onPageChanged(int id)
{
  switch (id) {
    case PageChooseFunction:
      powerRadio->setVisible(
          unusedDeviceTypes.contains(
              Enum<StateMachineDeviceType>::toQString(
                  StateMachineDeviceType::Power)));
      inputRadio->setVisible(
          unusedDeviceTypes.contains(
              Enum<StateMachineDeviceType>::toQString(
                  StateMachineDeviceType::Input)));
      elseRadio->setVisible(!unusedDeviceTypes.isEmpty());
      break;
    case PageDefineInputStates:
      if (discreteRadio->isChecked()) {
        pageInputStates->setSubTitle(tr("List the inputs your device has.\n"
            "It is recommended to add all available inputs to simplify "
            "changing your setup in the future. The order doesn't matter.\n\n"
            "If you run this for the first time, the list below  will contain "
            "all IR commands with \"input\" in the name"));
      } else if (relativeRadio->isChecked()) {
        pageInputStates->setSubTitle(tr("List the inputs your device has.\nFor "
            "cycle control to work correctly, you need to add all available "
            "inputs in the order given by your device.\n\nIf you run this for"
            "the first time, the list below  will contain all IR commands with "
            "\"input\" in the name"));
      } else {
        pageInputStates->setSubTitle(tr("List the inputs your device has."));
      }
      break;
    case PageAssignCommands:
      commandsTable->setHorizontalHeaderLabels( { tr("State"), tr("Command") });
      commandsTable->setRowCount(statesList->count());
      for (int i = 0; i < statesList->count(); i++) {
        QListWidgetItem *stateItem = statesList->item(i);

        if (stateItem != nullptr) {
          auto text = stateItem->text();
          QTableWidgetItem *nameItem = new QTableWidgetItem(text);
          nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
          commandsTable->setItem(i, 0, nameItem);
          QComboBox *commandCombo = new QComboBox(commandsTable);
          fillComboBox(commandCombo, text);
          commandsTable->setCellWidget(i, 1, commandCombo);
        }
      }
      break;
    case PageReview:
      reviewSummaryLabel->setText(reviewSummaryText());
      break;
    default:
      break;
  }
}

void StateMachineWizard::buildChooseTypePage()
{
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("Choose type"));
  page->setSubTitle(tr("This determines how your device is controlled."));

  discreteRadio = new QRadioButton(tr("Direct selection -- a button on "
      "the remote for each function, e.g. one button for \"On\" and a "
      "different button for \"Off\""), page);
  relativeRadio = new QRadioButton(tr("Cycle -- one button on the "
      "remote to toggle functions, e.g. power button for \"On\" and \"Off\"."),
      page);
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

void StateMachineWizard::buildChooseFunctionPage()
{
  QWizardPage *page = new ChooseFunctionPage(*this, this);

  unusedDeviceTypes = Enum<StateMachineDeviceType>::toQStringList();
  for (const auto &sm : device.getStateMachines()) {
    unusedDeviceTypes.removeAll(sm.smType.get().getQString());
  }
  //list might be empty!

  page->setTitle(tr("Choose function"));
  page->setSubTitle(tr("This determines what will be controlled."));

  powerRadio = new QRadioButton(
      tr("Power -- this will turn your device on and off"), page);
  inputRadio = new QRadioButton(
      tr("Input -- this will switch between the inputs"), page);
  elseRadio = new QRadioButton(
      tr("Something else -- this will create an empty control allowing "
          "manual config"), page);

  powerRadio->setChecked(true);

  QButtonGroup *group = new QButtonGroup(page);
  group->addButton(powerRadio);
  group->addButton(inputRadio);
  group->addButton(elseRadio);

  QLabel *delayLabel = new QLabel(tr("Time needed to complete (ms):"), page);
  delaySpinBox = new QSpinBox(page);
  delaySpinBox->setRange(0, 120000);
  delaySpinBox->setSingleStep(100);
  delaySpinBox->setValue(POWER_ON_DELAY_ms);
  delaySpinBox->setSuffix(tr(" ms"));

  connect(powerRadio, &QRadioButton::toggled, [this](bool checked) {
    if (checked) {
      delaySpinBox->setValue(POWER_ON_DELAY_ms);
    }
  });
  connect(inputRadio, &QRadioButton::toggled, [this](bool checked) {
    if (checked) {
      delaySpinBox->setValue(INPUT_SWITCH_DELAY_ms);
    }
  });
  connect(elseRadio, &QRadioButton::toggled, [this](bool checked) {
    if (checked) {
      delaySpinBox->setValue(INPUT_SWITCH_DELAY_ms); //use same as input
    }
  });

  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addWidget(powerRadio);
  layout->addWidget(inputRadio);
  layout->addWidget(elseRadio);
  layout->addWidget(delayLabel);
  layout->addWidget(delaySpinBox);
  layout->addStretch(1);

  setPage(PageChooseFunction, page);
}

void StateMachineWizard::buildDefineInputStatesPage()
{
  QWizardPage *page = new DefineInputsPage(*this, this);
  page->setTitle(tr("Define inputs"));
  //set in "onPageChanged"

  statesList = new QListWidget(page);
  statesList->setDragDropMode(QAbstractItemView::InternalMove);
  statesList->setEditTriggers(
      QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
  //best guess at pre-selection

  auto preSelection = lib::InputKeywordMatcher().filter(availableCommands);
  for (const auto &state : preSelection) {
    QListWidgetItem *item = new QListWidgetItem(state, statesList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }

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

  setPage(PageDefineInputStates, page);
  pageInputStates = page;
}

void StateMachineWizard::buildAssignCommandsPage()
{
  QWizardPage *page = new AssignCommandsPage(*this, this);
  page->setTitle(tr("Assign command"));
  page->setSubTitle(tr("Select the IR commands that will be used"));

  commandsTable = new QTableWidget(page);
  commandsTable->setColumnCount(2);
  commandsTable->horizontalHeader()->setStretchLastSection(true);
  commandsTable->verticalHeader()->setVisible(false);

  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addWidget(commandsTable);
  page->setLayout(layout);

  setPage(PageAssignCommands, page);
}

void StateMachineWizard::buildSetupCommandsPage()
{
  QWizardPage *page = new QWizardPage(this);

  page->setTitle(tr("Setup commands"));
  page->setSubTitle(tr("Select the IR commands used for switching between "
      "the states"));

  // Start command
  QGroupBox *startCommandGroup = new QGroupBox(tr("Start command. Optional, "
      "a button you need to press to open the selection, like \"Inputs\"."),
      page);
  startCommandEnabled = new QCheckBox(tr("Enabled"), startCommandGroup);
  QLabel *startCommandLabel = new QLabel(tr("Command"), startCommandGroup);
  startCommandCombo = new QComboBox(startCommandGroup);
  fillComboBox(startCommandCombo, "select input");
  startCommandCombo->setEnabled(false);

  QHBoxLayout *startCommandLayout = new QHBoxLayout(startCommandGroup);
  startCommandLayout->addWidget(startCommandEnabled);
  startCommandLayout->addWidget(startCommandLabel);
  startCommandLayout->addWidget(startCommandCombo);
  startCommandLayout->addStretch();

  connect(startCommandEnabled, &QCheckBox::toggled, startCommandCombo,
      &QComboBox::setEnabled);

  // Next state
  QGroupBox *nextStateGroup = new QGroupBox(tr("Next. Switches to next state"),
      page);
  QLabel *nextStateLabel = new QLabel(tr("Command"), nextStateGroup);
  nextStateCombo = new QComboBox(nextStateGroup);
  fillComboBox(nextStateCombo, "power toggle");

  QHBoxLayout *nextStateLayout = new QHBoxLayout(nextStateGroup);
  nextStateLayout->addWidget(nextStateLabel);
  nextStateLayout->addWidget(nextStateCombo);
  nextStateLayout->addStretch();

  // Previous state
  QGroupBox *previousStateGroup = new QGroupBox(
      tr("Previous. Optional, Switches to previous state"), page);
  previousStateEnabled = new QCheckBox(tr("Enabled"), previousStateGroup);
  QLabel *previousStateLabel = new QLabel(tr("Command"), previousStateGroup);
  previousStateCombo = new QComboBox(previousStateGroup);
  fillComboBox(previousStateCombo, "previous");
  previousStateCombo->setEnabled(false);

  QHBoxLayout *previousStateLayout = new QHBoxLayout(previousStateGroup);
  previousStateLayout->addWidget(previousStateEnabled);
  previousStateLayout->addWidget(previousStateLabel);
  previousStateLayout->addWidget(previousStateCombo);
  previousStateLayout->addStretch();

  connect(previousStateEnabled, &QCheckBox::toggled, previousStateCombo,
      &QComboBox::setEnabled);

  // Finish command
  QGroupBox *finishCommandGroup = new QGroupBox(tr("Finish command. Optional, "
      "a button you need to press to confirm the selection, like \"OK\"."),
      page);
  finishCommandEnabled = new QCheckBox(tr("Enabled"), finishCommandGroup);
  QLabel *finishCommandLabel = new QLabel(tr("Command"), finishCommandGroup);
  finishCommandCombo = new QComboBox(finishCommandGroup);
  fillComboBox(finishCommandCombo, "ok");
  finishCommandCombo->setEnabled(false);

  QHBoxLayout *finishCommandLayout = new QHBoxLayout(finishCommandGroup);
  finishCommandLayout->addWidget(finishCommandEnabled);
  finishCommandLayout->addWidget(finishCommandLabel);
  finishCommandLayout->addWidget(finishCommandCombo);
  finishCommandLayout->addStretch();

  connect(finishCommandEnabled, &QCheckBox::toggled, finishCommandCombo,
      &QComboBox::setEnabled);

  // Page layout
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addWidget(startCommandGroup);
  layout->addWidget(nextStateGroup);
  layout->addWidget(previousStateGroup);
  layout->addWidget(finishCommandGroup);
  layout->addStretch();

  page->setLayout(layout);

  setPage(PageSetupCommands, page);
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

QString StateMachineWizard::reviewSummaryText() const
{
  QString typeName;
  QString smName;
  QString stateNames;

  if (discreteRadio->isChecked()) {
    typeName = tr("Direct Selection");
  } else if (relativeRadio->isChecked()) {
    typeName = tr("Cycle based");
  } else {
    typeName = tr("Number based");
  }
  if (inputRadio->isChecked()) {
    smName = tr("Input selection");
  } else if (powerRadio->isChecked()) {
    smName = tr("Power handling");
  } else {
    return tr("You've set up a \"%1\" device control state machine for manual "
        "setup. \n\nClick \"Finish\" to apply the data. Any previously "
        "existing data will be overwritten!").arg(typeName);
  }
  for (int i = 0; i < statesList->count(); i++) {
    stateNames.push_back(statesList->item(i)->text() + " ");
  }
  if (stateNames.isEmpty()) {
    stateNames.push_back(tr("None"));
  }

  return tr("You've set up a \"%1\" device control state machine for %2. You "
      "have the following states:\n\n  %3\n\nClick \"Finish\" to "
      "apply the data. Any previously existing data will be overwritten!\n\n"
      "If additional button presses and/or delay times are needed for your "
      "device, you can manually add them after the data has been applied.").arg(
      typeName).arg(smName).arg(stateNames);
}

void StateMachineWizard::fillComboBox(QComboBox *box, QString text)
{
  box->clear();
  box->addItems(availableCommands);
  text = lib::BestMatchFinder().findBestMatch(text, availableCommands);
  if (text.isEmpty()) {
    box->addItem("");
  }
  box->setCurrentText(text);
}

//the following functions implement the "wizard state machine"
// --start--+--discrete--+--power---------------+--assigncmd-+-finish
//          |            +--input----addstates--+            |
//          |            +--else-----------------------------+
//          |                                                |
//          +--relative--+--power---------------+--setupcmd--+
//                       +--input----addstates--+            |
//          |            +--else-----------------------------+

int ChooseFunctionPage::nextId() const
{
  if (w.powerRadio->isChecked()) {
    if (w.discreteRadio->isChecked()) {
      return StateMachineWizard::PageId::PageAssignCommands;
    } else {
      return StateMachineWizard::PageId::PageSetupCommands;
    }
  } else if (w.inputRadio->isChecked()) {
    return StateMachineWizard::PageId::PageDefineInputStates;
  } else {
    return StateMachineWizard::PageId::PageReview;
  }
}

bool editors::ChooseFunctionPage::validatePage()
{
  if (w.powerRadio->isChecked()) {
    //power uses fixed labels, assign those
    w.statesList->clear();
    w.statesList->addItems(powerItems);
  }
  return true;
}

int DefineInputsPage::nextId() const
{
  if (w.discreteRadio->isChecked()) {
    return StateMachineWizard::PageId::PageAssignCommands;
  } else {
    return StateMachineWizard::PageId::PageSetupCommands;
  }
}

int editors::AssignCommandsPage::nextId() const
{
  return StateMachineWizard::PageId::PageReview;
}

bool editors::AssignCommandsPage::validatePage()
{
  bool allCommandsSelected = true;

  for (int row = 0; row < w.commandsTable->rowCount(); row++) {
    QComboBox *comboBox = qobject_cast<QComboBox*>(
        w.commandsTable->cellWidget(row, 1));
    if ((comboBox != nullptr) && comboBox->currentText().isEmpty()) {
      allCommandsSelected = false;
      break;
    }
  }
  if (allCommandsSelected) {
    return true;
  } else {
    QMessageBox msgBox(QMessageBox::Warning, tr("Missing selection"),
        tr("At least one state has no command assigned. Can't continue."),
        QMessageBox::Ok);
    msgBox.exec();
    return false;
  }
}

}
