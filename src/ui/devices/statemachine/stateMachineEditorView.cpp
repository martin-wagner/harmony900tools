// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QMessageBox>
#include <QTreeView>
#include <QLabel>

#include "models/statemachineListModel.h"
#include "ui/delegates/combobox.h"
#include "stateMachineEditorView.h"

//#include "stateMachineEditorView.h"
//#include "stateMachineListWidget.h"
//#include "stateMachineDetailPanel.h"
//#include "stateMachineWizard.h"

using namespace std;

namespace editors
{
//
//StateMachineEditorView::StateMachineEditorView(QWidget *parent) :
//    QWidget(parent)
//{
//  buildUi();
//}
//
//void StateMachineEditorView::setStateMachines(
//    const QVector<StateMachine> &newStateMachines)
//{
//  stateMachines = newStateMachines;
//  currentRow = -1;
//
//  listWidget->setStateMachines(stateMachines);
//  detailPanel->showEmptyState();
//}
//
//QVector<StateMachine> StateMachineEditorView::getStateMachines() const
//{
//  return stateMachines;
//}
//
//void StateMachineEditorView::onCurrentRowChanged(int row)
//{
//  storeCurrentDetailEdits();
//
//  currentRow = row;
//
//  if (row < 0 || row >= stateMachines.size()) {
//    detailPanel->showEmptyState();
//    return;
//  }
//
//  detailPanel->setStateMachine(stateMachines.at(row));
//}
//
//void StateMachineEditorView::onAddRequested()
//{
//  StateMachineWizard wizard(this);
//
//  if (wizard.exec() == QDialog::Accepted) {
//    stateMachines.append(wizard.getStateMachine());
//    listWidget->setStateMachines(stateMachines);
//    listWidget->setCurrentRow(stateMachines.size() - 1);
//  }
//}
//
//void StateMachineEditorView::onRerunWizardRequested()
//{
//  if (currentRow < 0 || currentRow >= stateMachines.size()) {
//    return;
//  }
//
//  StateMachineWizard wizard(this);
//  wizard.setStateMachine(stateMachines.at(currentRow));
//
//  if (wizard.exec() == QDialog::Accepted) {
//    stateMachines[currentRow] = wizard.getStateMachine();
//    listWidget->setStateMachines(stateMachines);
//    listWidget->setCurrentRow(currentRow);
//  }
//}
//
//void StateMachineEditorView::onDeleteRequested()
//{
//  if (currentRow < 0 || currentRow >= stateMachines.size()) {
//    return;
//  }
//
//  const QMessageBox::StandardButton answer = QMessageBox::question(this,
//      tr("Delete state machine"),
//      tr("Remove this state machine? This cannot be undone."));
//
//  if (answer != QMessageBox::Yes) {
//    return;
//  }
//
//  stateMachines.remove(currentRow);
//  currentRow = -1;
//
//  listWidget->setStateMachines(stateMachines);
//  detailPanel->showEmptyState();
//}
//
//void StateMachineEditorView::onDetailChanged()
//{
//  storeCurrentDetailEdits();
//}
//
//void StateMachineEditorView::buildUi()
//{
//  listWidget = new StateMachineListWidget(this);
//  connect(listWidget, &StateMachineListWidget::currentChanged, this,
//      &StateMachineEditorView::onCurrentRowChanged);
//  connect(listWidget, &StateMachineListWidget::addRequested, this,
//      &StateMachineEditorView::onAddRequested);
//
//  detailPanel = new StateMachineDetailPanel(this);
//  connect(detailPanel, &StateMachineDetailPanel::changed, this,
//      &StateMachineEditorView::onDetailChanged);
//  connect(detailPanel, &StateMachineDetailPanel::deleteRequested, this,
//      &StateMachineEditorView::onDeleteRequested);
//  connect(detailPanel, &StateMachineDetailPanel::rerunWizardRequested, this,
//      &StateMachineEditorView::onRerunWizardRequested);
//
//  QHBoxLayout *rootLayout = new QHBoxLayout(this);
//  rootLayout->addWidget(listWidget, 0);
//  rootLayout->addWidget(detailPanel, 1);
//}
//
//void StateMachineEditorView::storeCurrentDetailEdits()
//{
//  if (currentRow < 0 || currentRow >= stateMachines.size()) {
//    return;
//  }
//
//  stateMachines[currentRow] = detailPanel->getStateMachine();
//}

StateMachineTreeView::StateMachineTreeView(Context &ctx, QWidget *parent) :
    BaseTreeView(ctx, "", false, parent)
{
  setupDelegates();
}

StateMachineTreeView::~StateMachineTreeView() = default;

void StateMachineTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);
}

void StateMachineTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  treeView->setItemDelegateForColumn(
      models::StateMachineModel::Column::CONTROL_TYPE, comboBoxDelegate);
}

StateMachineEditorView::StateMachineEditorView(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx)
{
  createView();
  setupTreeView();
  createConnections();
}

StateMachineEditorView::~StateMachineEditorView() = default;

void StateMachineEditorView::addTreeView(StateMachineTreeView *stateTree)
{
  layout->addWidget(stateTree, 0);
  //rootLayout->addWidget(detailPanel, 1);
}

void StateMachineEditorView::setData(uint32_t deviceId, QAbstractItemModel *stateModel)
{
  if (stateModel == nullptr) {
    this->deviceId = 0;
    return;
  }
  this->deviceId = deviceId;

  //pull data from document
  onStateMachineDataChanged(document::data::Item::DEVICE_STATEMACHINE, 0);
}

void StateMachineEditorView::onStateMachineDataChanged(document::data::Item item,
    uint32_t pos)
{
  if (item != document::data::Item::DEVICE_STATEMACHINE) {
    return;
  }

  auto *device = ctx.config()->data().getDevice(deviceId);
  if (device == nullptr) {
    return;
  }

//todo update non-model/view data
//  auto &commands = device->getIrCommands();
//  pressPreSilenceSpinBox->setValue(commands.pressPreSilenceMs.get());
//  holdPreSilencSpinBox->setValue(commands.holdPreSilenceMs.get());
//  interKeySpinBox->setValue(
//      max(commands.pressInterKeyMs.get(), commands.holdInterKeyMs.get()));
}

void StateMachineEditorView::createView()
{
  auto baseLayout = new QVBoxLayout(this);
  baseLayout->setContentsMargins(0, 0, 0, 0);
  baseLayout->setSpacing(0);

  header = new QLabel("Device control", this);
  auto font = header->font();
  font.setBold(true);
  header->setFont(font);
  baseLayout->addWidget(header);

  //fixme use ads for this
  layout = new QHBoxLayout(this);
//  layout->setContentsMargins(0, 0, 0, 0);
//  layout->setSpacing(0); todo
  baseLayout->addLayout(layout);
}

void StateMachineEditorView::setupTreeView()
{
}

void StateMachineEditorView::createConnections()
{
  connect(ctx.config(), &document::Config::itemChanged, this,
      &StateMachineEditorView::onStateMachineDataChanged);
}

}
