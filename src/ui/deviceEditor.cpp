// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAction>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "lib/icon.h"
#include "deviceEditor.h"
#include "models/deviceListModel.h"

using namespace std;

namespace editors
{

DeviceEditor::DeviceEditor(Context &ctx, models::DeviceModel *model,
    QWidget *parent) :
    ctx(ctx)
{
  createView();
  createActions();
  setModel(model);
}

DeviceEditor::~DeviceEditor() = default;

void DeviceEditor::setModel(models::DeviceModel *model)
{
  if (model != nullptr) {
    disconnect(model, &QAbstractItemModel::rowsInserted, this,
        &DeviceEditor::onModelRowCountChanged);
    disconnect(model, &QAbstractItemModel::rowsRemoved, this,
        &DeviceEditor::onModelRowCountChanged);
  }

  if (treeView->selectionModel() != nullptr) {
    disconnect(treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged, this,
        &DeviceEditor::onViewSelectionChanged);
  }

  this->model = model;
  treeView->setModel(model);

  if (model != nullptr) {
    connect(model, &QAbstractItemModel::rowsInserted, this,
        &DeviceEditor::onModelRowCountChanged);
    connect(model, &QAbstractItemModel::rowsRemoved, this,
        &DeviceEditor::onModelRowCountChanged);
  }

  if (treeView->selectionModel() != nullptr) {
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &DeviceEditor::onViewSelectionChanged);
  }

  updateActions();
  onSettingsChanged();
  onUserLevelChanged(ctx.userLevel().getLevel());
  emit selectionChanged(-1, 0);
}

void DeviceEditor::onViewSelectionChanged(const QItemSelection &selected,
    const QItemSelection &deselected)
{
  Q_UNUSED(deselected)

  updateActions();

  if (selected.isEmpty()) {
    emit selectionChanged(-1, 0);
    return;
  }

  auto row = selected.indexes().first().row();
  auto idIndex = model->index(row, models::DeviceModel::Column::ID);
  auto deviceId = model->data(idIndex, Qt::DisplayRole).toUInt();

  emit selectionChanged(row, deviceId);
}

void DeviceEditor::onAddDevice()
{
  if (model == nullptr) {
    return;
  }
  model->insertRows(getCurrentRow() + 1, 1);
}

void DeviceEditor::onRemoveDevice()
{
  if (model == nullptr) {
    return;
  }
  const int row = getCurrentRow();
  if (row < 0) {
    return;
  }
  model->removeRows(row, 1);
}

void DeviceEditor::onMoveUp()
{
  if (model == nullptr) {
    return;
  }
  const int row = getCurrentRow();
  if (row <= 0) {
    return;
  }
  model->moveRows(QModelIndex(), row, 1, QModelIndex(), row - 1);
}

void DeviceEditor::onMoveDown()
{
  if (model == nullptr) {
    return;
  }
  const int row = getCurrentRow();
  if (row < 0 || row >= model->rowCount() - 1) {
    return;
  }
  model->moveRows(QModelIndex(), row, 1, QModelIndex(), row + 2);
}

void DeviceEditor::onModelRowCountChanged()
{
  updateActions();
}

void DeviceEditor::onUserLevelChanged(lib::UserLevel::Level l)
{
  if (lib::UserLevel::validate(l, lib::UserLevel::Level::Developer)) {
    treeView->showColumn(models::DeviceModel::Column::ID);
  } else {
    treeView->hideColumn(models::DeviceModel::Column::ID);
  }
}

void DeviceEditor::onSettingsChanged()
{
}

void DeviceEditor::createView()
{
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  setupToolbar();
  layout->addWidget(toolbar);

  setupTreeView();
  layout->addWidget(treeView);
}

void DeviceEditor::setupToolbar()
{
  toolbar = new QToolBar(this);
  //toolbar->setIconSize(QSize(16, 16));
  toolbar->setFloatable(false);
  toolbar->setMovable(false);

  actionAdd = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/list-add.png",
          "list-add"), tr("Add"));
  actionAdd->setToolTip(tr("Add a new device"));

  actionRemove = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
          "edit-delete"), tr("Remove"));
  actionRemove->setToolTip(tr("Remove the selected device"));

  toolbar->addSeparator();

  actionMoveUp = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-up.png",
          "go-up"), tr("Move Up"));
  actionMoveUp->setToolTip(tr("Move the selected device one position up"));

  actionMoveDown = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-down.png",
          "go-down"), tr("Move Down"));
  actionMoveDown->setToolTip(tr("Move the selected device one position down"));
}

void DeviceEditor::setupTreeView()
{
  treeView = new QTreeView(this);

  treeView->setRootIsDecorated(false);
  treeView->setAlternatingRowColors(true);
  treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeView->setUniformRowHeights(true);
  treeView->setDragDropMode(QAbstractItemView::NoDragDrop);
  treeView->header()->setStretchLastSection(true);
  treeView->header()->setSectionResizeMode(QHeaderView::Interactive);

  treeView->setEditTriggers(
      QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

int DeviceEditor::getCurrentRow() const
{
  const QModelIndexList selected = treeView->selectionModel()->selectedRows();
  if (selected.isEmpty()) {
    return -1;
  }
  return selected.first().row();
}

void DeviceEditor::createActions()
{
  connect(actionAdd, &QAction::triggered, this, &DeviceEditor::onAddDevice);
  connect(actionRemove, &QAction::triggered, this,
      &DeviceEditor::onRemoveDevice);
  connect(actionMoveUp, &QAction::triggered, this, &DeviceEditor::onMoveUp);
  connect(actionMoveDown, &QAction::triggered, this, &DeviceEditor::onMoveDown);

  connect(&ctx.userLevel(), &lib::UserLevel::levelChanged, this,
      &DeviceEditor::onUserLevelChanged);
  connect(&ctx.settings(), &Settings::settingsAccepted, this,
      &DeviceEditor::onSettingsChanged);
}

void DeviceEditor::updateActions()
{
  int rowCount = 0;
  if (model != nullptr) {
    rowCount = model->rowCount();
  }

  const int row = getCurrentRow();
  const bool hasSelection = (model != nullptr) && (row >= 0);

  actionRemove->setEnabled(hasSelection);
  actionMoveUp->setEnabled(hasSelection && row > 0);
  actionMoveDown->setEnabled(hasSelection && row < rowCount - 1);
}

} // namespace views
