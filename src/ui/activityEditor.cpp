// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAction>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "lib/icon.h"
#include "activityEditor.h"
#include "models/activityListModel.h"
#include "delegates/combobox.h"

using namespace std;

namespace editors
{

ActivityEditor::ActivityEditor(Context &ctx, models::ActivityModel *model,
    QWidget *parent) :
    ctx(ctx)
{
  createView();
  createActions();
  setModel(model);
}

ActivityEditor::~ActivityEditor() = default;

void ActivityEditor::setModel(models::ActivityModel *model)
{
  if (model != nullptr) {
    disconnect(model, &QAbstractItemModel::rowsInserted, this,
        &ActivityEditor::onModelRowCountChanged);
    disconnect(model, &QAbstractItemModel::rowsRemoved, this,
        &ActivityEditor::onModelRowCountChanged);
  }

  if (treeView->selectionModel() != nullptr) {
    disconnect(treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged, this,
        &ActivityEditor::onViewSelectionChanged);
  }

  this->model = model;
  treeView->setModel(model);

  if (model != nullptr) {
    connect(model, &QAbstractItemModel::rowsInserted, this,
        &ActivityEditor::onModelRowCountChanged);
    connect(model, &QAbstractItemModel::rowsRemoved, this,
        &ActivityEditor::onModelRowCountChanged);
  }

  if (treeView->selectionModel() != nullptr) {
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ActivityEditor::onViewSelectionChanged);
  }

  updateActions();
  onSettingsChanged();
  onUserLevelChanged(ctx.userLevel().getLevel());
  emit selectionChanged(-1, 0);
}

void ActivityEditor::onViewSelectionChanged(const QItemSelection &selected,
    const QItemSelection &deselected)
{
  Q_UNUSED(deselected)

  updateActions();

  if (selected.isEmpty()) {
    emit selectionChanged(-1, 0);
    return;
  }

  auto row = selected.indexes().first().row();
  auto idIndex = model->index(row, models::ActivityModel::Column::ID);
  auto activityId = model->data(idIndex, Qt::DisplayRole).toUInt();

  emit selectionChanged(row, activityId);
}

void ActivityEditor::onAddActivity()
{
  if (model == nullptr) {
    return;
  }
  model->insertRows(treeView->model()->rowCount(), 1);
}

void ActivityEditor::onRemoveActivity()
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

void ActivityEditor::onMoveUp()
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

void ActivityEditor::onMoveDown()
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

void ActivityEditor::onModelRowCountChanged()
{
  updateActions();
}

void ActivityEditor::onUserLevelChanged(lib::UserLevel::Level l)
{
  if (lib::UserLevel::validate(l, lib::UserLevel::Level::Developer)) {
    treeView->showColumn(models::ActivityModel::Column::ID);
  } else {
    treeView->hideColumn(models::ActivityModel::Column::ID);
  }
}

void ActivityEditor::onSettingsChanged()
{
}

void ActivityEditor::createView()
{
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  setupToolbar();
  layout->addWidget(toolbar);

  setupTreeView();
  layout->addWidget(treeView);
}

void ActivityEditor::setupToolbar()
{
  toolbar = new QToolBar(this);
  //toolbar->setIconSize(QSize(16, 16));
  toolbar->setFloatable(false);
  toolbar->setMovable(false);

  actionAdd = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/list-add.png",
          "list-add"), tr("Add"));
  actionAdd->setToolTip(tr("Add a new activity"));

  actionRemove = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
          "edit-delete"), tr("Remove"));
  actionRemove->setToolTip(tr("Remove the selected activity"));
  actionRemove->setShortcut(QKeySequence::Delete);
  actionRemove->setShortcutContext(Qt::WidgetShortcut);

  toolbar->addSeparator();

  actionMoveUp = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-up.png",
          "go-up"), tr("Move Up"));
  actionMoveUp->setToolTip(tr("Move the selected activity one position up"));

  actionMoveDown = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-down.png",
          "go-down"), tr("Move Down"));
  actionMoveDown->setToolTip(tr("Move the selected activity one position down"));
}

void ActivityEditor::setupTreeView()
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
  treeView->addAction(actionRemove); //for delete shortcut

  auto *comboBoxDelegate = new delegates::ComboBox(this);
  treeView->setItemDelegateForColumn(models::ActivityModel::Column::ACTTYPE,
      comboBoxDelegate);

  treeView->setEditTriggers(
      QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

int ActivityEditor::getCurrentRow() const
{
  const QModelIndexList selected = treeView->selectionModel()->selectedRows();
  if (selected.isEmpty()) {
    return -1;
  }
  return selected.first().row();
}

void ActivityEditor::createActions()
{
  connect(actionAdd, &QAction::triggered, this, &ActivityEditor::onAddActivity);
  connect(actionRemove, &QAction::triggered, this,
      &ActivityEditor::onRemoveActivity);
  connect(actionMoveUp, &QAction::triggered, this, &ActivityEditor::onMoveUp);
  connect(actionMoveDown, &QAction::triggered, this, &ActivityEditor::onMoveDown);

  connect(&ctx.userLevel(), &lib::UserLevel::levelChanged, this,
      &ActivityEditor::onUserLevelChanged);
  connect(&ctx.settings(), &Settings::settingsAccepted, this,
      &ActivityEditor::onSettingsChanged);
}

void ActivityEditor::updateActions()
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
