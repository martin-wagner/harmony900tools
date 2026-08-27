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
  //no inheritance in constructor!
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

  actionAdd = toolbar->addAction(lib::getAddIcon(), tr("Add"));
  actionAdd->setToolTip(tr("Add a new item to the selected list. Click into\n"
      "the empty space to select for adding the first item."));

  actionRemove = toolbar->addAction(lib::getDeleteIcon(), tr("Remove"));
  actionRemove->setToolTip(tr("Remove the selected item"));
  actionRemove->setShortcut(QKeySequence::Delete);
  actionRemove->setShortcutContext(Qt::ApplicationShortcut);

  toolbar->addSeparator();

  actionMoveUp = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-up.png",
          "go-up"), tr("Move Up"));
  actionMoveUp->setToolTip(tr("Move the selected item one position up"));

  actionMoveDown = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/go-down.png",
          "go-down"), tr("Move Down"));
  actionMoveDown->setToolTip(tr("Move the selected item one position down"));

  toolbar->addSeparator();

  actionImport = toolbar->addAction(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/document-import.png"),
      tr("Import device"));
  actionImport->setToolTip(tr("Import an existing item"));
  actionImport->setVisible(false);
  actionExport = toolbar->addAction(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/document-export.png"),
      tr("Export device"));
  actionExport->setToolTip(
      tr("Export the selected item to a file for re-use / sharing"));
  actionExport->setVisible(false);
}

void BaseEditor::createConnections()
{
  connect(actionAdd, &QAction::triggered, this, &BaseEditor::onAddClicked);
  connect(actionRemove, &QAction::triggered, this,
      &BaseEditor::onRemoveClicked);
  connect(actionMoveUp, &QAction::triggered, this,
      &BaseEditor::onMoveUpClicked);
  connect(actionMoveDown, &QAction::triggered, this,
      &BaseEditor::onMoveDownClicked);
  connect(actionExport, &QAction::triggered, this,
      &BaseEditor::onExportClicked);
  connect(actionImport, &QAction::triggered, this,
      &BaseEditor::onImportClicked);

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
  actionAdd->setEnabled(true);
  actionImport->setEnabled(true);

  if (lastActiveView == nullptr) {
    actionRemove->setEnabled(false);
    actionMoveUp->setEnabled(false);
    actionMoveDown->setEnabled(false);
    actionExport->setEnabled(false);
    return;
  }
  actionRemove->setEnabled(lastActiveView->canRemove());

  auto move = lastActiveView->availableMoveOperations();
  switch (move) {
    case BaseTreeView::MoveOperation::None:
      actionMoveUp->setEnabled(false);
      actionMoveDown->setEnabled(false);
      break;
    case BaseTreeView::MoveOperation::Up:
      actionMoveUp->setEnabled(true);
      actionMoveDown->setEnabled(false);
      break;
    case BaseTreeView::MoveOperation::Down:
      actionMoveUp->setEnabled(false);
      actionMoveDown->setEnabled(true);
      break;
    default:
      //no harm if enabled, lower level will just drop the request.
      actionMoveUp->setEnabled(true);
      actionMoveDown->setEnabled(true);
      break;
  }

  if (lastActiveView == mainView) {
    actionExport->setEnabled(true);
  } else {
    actionExport->setEnabled(false);

  }
}

void BaseEditor::addHLine(QSplitter *s)
{
  auto *separator = new QFrame(s);
  separator->setFrameShape(QFrame::HLine);
  separator->setFrameShadow(QFrame::Sunken);
  s->addWidget(separator);
}

} // namespace editors
