// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QTreeView>
#include <QLabel>

#include "models/statemachineListModel.h"
#include "ui/delegates/combobox.h"
#include "stateMachineEditorView.h"
#include "stateMachineDetailPanel.h"

using namespace std;

namespace editors
{

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

StateMachineEditorView::StateMachineEditorView(Context &ctx,
    StateMachineTreeView *stateTree, QWidget *parent) :
    QWidget(parent), ctx(ctx), tree(stateTree)
{
  createView();
  setupTreeView();
  createConnections();
}

StateMachineEditorView::~StateMachineEditorView() = default;

void StateMachineEditorView::setData(uint32_t deviceId,
    QAbstractItemModel *stateModel)
{
  if (stateModel == nullptr) {
    this->deviceId = 0;
    return;
  }
  this->deviceId = deviceId;

  //pull data from document
  onStateMachineDataChanged(document::data::Item::DEVICE_STATEMACHINE, 0);
}

void StateMachineEditorView::onStateMachineSelectionChanged(int row)
{
  uint32_t devicePos;

  if (row < 0) {
    detailPanel->showEmptyState();
    return;
  }

  auto *device = ctx.config()->data().getDevice(deviceId, &devicePos);
  if (device == nullptr) {
    detailPanel->showEmptyState();
    return;
  }
  auto &statemachines = device->getStateMachines();
  if (row >= statemachines.size()) {
    detailPanel->showEmptyState();
    return;
  }
  detailPanel->setStateMachine(devicePos, row);
}

void StateMachineEditorView::onStateMachineDataChanged(
    document::data::Item item, uint32_t pos)
{
  if (item != document::data::Item::DEVICE_STATEMACHINE) {
    return;
  }
  detailPanel->updateData();
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
  layout = new QHBoxLayout();
  baseLayout->addLayout(layout);
  layout->addWidget(tree);
  detailPanel = new StateMachineDetailPanel(ctx, this);
  layout->addWidget(detailPanel);
}

void StateMachineEditorView::setupTreeView()
{
}

void StateMachineEditorView::createConnections()
{
  connect(ctx.config(), &document::Config::itemChanged, this,
      &StateMachineEditorView::onStateMachineDataChanged);
  connect(tree, &StateMachineTreeView::selectionChanged, this,
      &StateMachineEditorView::onStateMachineSelectionChanged);
}

}
