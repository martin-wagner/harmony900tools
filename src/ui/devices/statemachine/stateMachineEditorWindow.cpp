// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QHBoxLayout>
#include <QMessageBox>

#include "stateMachineEditorWindow.h"
#include "stateMachineListWidget.h"
#include "stateMachineDetailPanel.h"
#include "addStateMachineWizard.h"

namespace document
{
namespace data
{
namespace item
{

StateMachineEditorWindow::StateMachineEditorWindow(QWidget *parent) :
    QWidget(parent)
{
  buildUi();
}

void StateMachineEditorWindow::setStateMachines(
    const QVector<StateMachine> &newStateMachines)
{
  stateMachines = newStateMachines;
  currentRow = -1;

  listWidget->setStateMachines(stateMachines);
  detailPanel->showEmptyState();
}

QVector<StateMachine> StateMachineEditorWindow::getStateMachines() const
{
  return stateMachines;
}

void StateMachineEditorWindow::onCurrentRowChanged(int row)
{
  storeCurrentDetailEdits();

  currentRow = row;

  if (row < 0 || row >= stateMachines.size()) {
    detailPanel->showEmptyState();
    return;
  }

  detailPanel->setStateMachine(stateMachines.at(row));
}

void StateMachineEditorWindow::onAddRequested()
{
  AddStateMachineWizard wizard(this);

  if (wizard.exec() == QDialog::Accepted) {
    stateMachines.append(wizard.getStateMachine());
    listWidget->setStateMachines(stateMachines);
    listWidget->setCurrentRow(stateMachines.size() - 1);
  }
}

void StateMachineEditorWindow::onRerunWizardRequested()
{
  if (currentRow < 0 || currentRow >= stateMachines.size()) {
    return;
  }

  AddStateMachineWizard wizard(this);
  wizard.setStateMachine(stateMachines.at(currentRow));

  if (wizard.exec() == QDialog::Accepted) {
    stateMachines[currentRow] = wizard.getStateMachine();
    listWidget->setStateMachines(stateMachines);
    listWidget->setCurrentRow(currentRow);
  }
}

void StateMachineEditorWindow::onDeleteRequested()
{
  if (currentRow < 0 || currentRow >= stateMachines.size()) {
    return;
  }

  const QMessageBox::StandardButton answer = QMessageBox::question(this,
      tr("Delete state machine"),
      tr("Remove this state machine? This cannot be undone."));

  if (answer != QMessageBox::Yes) {
    return;
  }

  stateMachines.remove(currentRow);
  currentRow = -1;

  listWidget->setStateMachines(stateMachines);
  detailPanel->showEmptyState();
}

void StateMachineEditorWindow::onDetailChanged()
{
  storeCurrentDetailEdits();
}

void StateMachineEditorWindow::buildUi()
{
  listWidget = new StateMachineListWidget(this);
  connect(listWidget, &StateMachineListWidget::currentChanged, this,
      &StateMachineEditorWindow::onCurrentRowChanged);
  connect(listWidget, &StateMachineListWidget::addRequested, this,
      &StateMachineEditorWindow::onAddRequested);

  detailPanel = new StateMachineDetailPanel(this);
  connect(detailPanel, &StateMachineDetailPanel::changed, this,
      &StateMachineEditorWindow::onDetailChanged);
  connect(detailPanel, &StateMachineDetailPanel::deleteRequested, this,
      &StateMachineEditorWindow::onDeleteRequested);
  connect(detailPanel, &StateMachineDetailPanel::rerunWizardRequested, this,
      &StateMachineEditorWindow::onRerunWizardRequested);

  QHBoxLayout *rootLayout = new QHBoxLayout(this);
  rootLayout->addWidget(listWidget, 0);
  rootLayout->addWidget(detailPanel, 1);
}

void StateMachineEditorWindow::storeCurrentDetailEdits()
{
  if (currentRow < 0 || currentRow >= stateMachines.size()) {
    return;
  }

  stateMachines[currentRow] = detailPanel->getStateMachine();
}

}
}
}
