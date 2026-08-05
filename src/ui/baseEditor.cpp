// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSplitter>

#include <DockManager.h>

#include "lib/icon.h"
#include "baseEditor.h"
#include "baseTreeView.h"

using namespace std;

namespace editors
{

BaseEditor::BaseEditor(Context &ctx, QWidget *parent) :
    ctx(ctx)
{
}

BaseEditor::~BaseEditor() = default;

void BaseEditor::onAddClicked()
{
  if (lastActiveView == nullptr) {
    return;
  }
  lastActiveView->addRow();
}

void BaseEditor::onRemoveClicked()
{
  if (lastActiveView == nullptr) {
    return;
  }
  lastActiveView->removeRow();
}

void BaseEditor::onMoveUpClicked()
{
  if (lastActiveView == nullptr) {
    return;
  }
  lastActiveView->moveUpRow();
}

void BaseEditor::onMoveDownClicked()
{
  if (lastActiveView == nullptr) {
    return;
  }
  lastActiveView->moveDownRow();
}

void BaseEditor::onAvailabilityChanged()
{
  updateActions();
}

void BaseEditor::createView()
{
  //fixme use ads for this
  layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  setupToolbar();
  layout->addWidget(toolbar);

  //override and implement the rest of your view
}

void BaseEditor::setupToolbar()
{
  toolbar = new QToolBar(this);
  //toolbar->setIconSize(QSize(16, 16));
  toolbar->setFloatable(false);
  toolbar->setMovable(false);

  actionAdd = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/list-add.png",
          "list-add"), tr("Add"));
  actionAdd->setToolTip(tr("Add a new entry to the selected list. Click into\n"
      "the empty space to select for adding the first item."));

  actionRemove = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
          "edit-delete"), tr("Remove"));
  actionRemove->setToolTip(tr("Remove the selected entry"));
  actionRemove->setShortcut(QKeySequence::Delete);
  actionRemove->setShortcutContext(Qt::ApplicationShortcut);

  toolbar->addSeparator();

  actionMoveUp = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-up.png",
          "go-up"), tr("Move Up"));
  actionMoveUp->setToolTip(tr("Move the selected activity one position up"));

  actionMoveDown = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-down.png",
          "go-down"), tr("Move Down"));
  actionMoveDown->setToolTip(
      tr("Move the selected activity one position down"));
}

void BaseEditor::createConnections()
{
  connect(actionAdd, &QAction::triggered, this, &BaseEditor::onAddClicked);
  connect(actionRemove, &QAction::triggered, this,
      &BaseEditor::onRemoveClicked);

  connect(mainView, &BaseTreeView::availabilityChanged, this,
      &BaseEditor::onAvailabilityChanged);
  connect(mainView, &BaseTreeView::selectionChanged, this,
      &BaseEditor::onSelectionChanged);
  connect(mainView, &BaseTreeView::activated, this, [this](BaseTreeView *view) {
    lastActiveView = view;
  });

  for (auto *childView : childViews) {
    connect(childView, &BaseTreeView::availabilityChanged, this,
        &BaseEditor::onAvailabilityChanged);
    connect(childView, &BaseTreeView::activated, this,
        [this](BaseTreeView *view) {
          lastActiveView = view;
        });
  }
}

void BaseEditor::updateActions()
{
  if (lastActiveView == nullptr) {
    actionAdd->setEnabled(false);
    actionRemove->setEnabled(false);
    return;
  }
  actionAdd->setEnabled(true);
  actionRemove->setEnabled(lastActiveView->canRemove());


//  int rowCount = 0;
//  if (model != nullptr) {
//    rowCount = model->rowCount();
//  }
//
//  const int row = getCurrentRow();
//  const bool hasSelection = (model != nullptr) && (row >= 0);
//
//  actionRemove->setEnabled(hasSelection);
//  actionMoveUp->setEnabled(hasSelection && row > 0);
//  actionMoveDown->setEnabled(hasSelection && row < rowCount - 1);
}

} // namespace editors
