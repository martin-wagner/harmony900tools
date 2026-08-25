// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>

#include "baseTreeView.h"
#include "defaults.h"

using namespace std;

namespace editors
{

BaseTreeView::BaseTreeView(Context &ctx, const QString &title, bool bold,
    QWidget *parent) :
    ctx(ctx)
{
  createView(title, bold);
  createConnections();
  createEventFilter();
  onSettingsChanged();
}

BaseTreeView::~BaseTreeView() = default;

void BaseTreeView::addRow()
{
  int row;

  if (model == nullptr) {
    return;
  }
  row = getCurrentRow();
  const int column = getCurrentColumn();
  if (row < 0) {
    row = treeView->model()->rowCount();
  } else {
    row = row + 1;
  }
  auto ret = model->insertRows(row, 1);
  if (ret == true) {
    auto newIndex = model->index(row, column);
    treeView->setCurrentIndex(newIndex);
  }
}

void BaseTreeView::removeRow()
{
  if (model == nullptr) {
    return;
  }
  const int row = getCurrentRow();
  const int column = getCurrentColumn();
  if (row < 0) {
    return;
  }
  auto ret = model->removeRows(row, 1);
  if (ret == true) {
    auto newIndex = model->index(row - 1, column);
    treeView->setCurrentIndex(newIndex);
  }
}

BaseTreeView::MoveOperation BaseTreeView::availableMoveOperations() const
{
  int rowCount = 0;

  if (!supportsMoveOperation() || (model == nullptr)) {
    return MoveOperation::None;
  }

  auto row = getCurrentRow();
  if (row < 0) {
    return MoveOperation::None;
  }
  rowCount = model->rowCount();
  if ((row > 0) && (row < (rowCount - 1))) {
    return MoveOperation::Both;
  }
  if (row > 0) {
    return MoveOperation::Up;
  }
  return MoveOperation::Down;
}

void BaseTreeView::moveUpRow()
{
  if (model == nullptr) {
    return;
  }
  const int row = getCurrentRow();
  const int column = getCurrentColumn();
  if (row <= 0) {
    return;
  }
  auto ret = model->moveRows(QModelIndex(), row, 1, QModelIndex(), row - 1);
  if (ret == true) {
    auto newIndex = model->index(row - 1, column);
    treeView->setCurrentIndex(newIndex);
  }
}

void BaseTreeView::moveDownRow()
{
  if (model == nullptr) {
    return;
  }
  const int row = getCurrentRow();
  const int column = getCurrentColumn();
  if (row < 0 || row >= model->rowCount() - 1) {
    return;
  }
  auto ret = model->moveRows(QModelIndex(), row, 1, QModelIndex(), row + 1);
  if (ret == true) {
    auto newIndex = model->index(row + 1, column);
    treeView->setCurrentIndex(newIndex);
  }
}

bool BaseTreeView::canRemove() const
{
  const int row = getCurrentRow();
  return (model != nullptr) && (row >= 0);
}

bool BaseTreeView::hasSelection() const
{
  const int row = getCurrentRow();
  if ((model == nullptr) || (row < 0)) {
    return false;
  }
  return true;
}

bool BaseTreeView::isActive() const
{
  return treeView->hasFocus();
}

void BaseTreeView::onViewSelectionChanged(const QItemSelection &selected,
    const QItemSelection &deselected)
{
  emit availabilityChanged();

  if (selected.isEmpty()) {
    emit selectionChanged(-1);
    return;
  }

  auto row = selected.indexes().first().row();
  emit selectionChanged(row);
}

void BaseTreeView::onModelRowCountChanged()
{
  emit availabilityChanged();
}

void BaseTreeView::onSettingsChanged()
{
  bool ok;

  auto factor = ctx.settings().value(defaults::columWithFactor().key).toDouble(
      &ok);
  if (ok) {
    treeView->header()->setDefaultSectionSize(
        factor * defaults::DEFAULT_COLUMN_WIDTH);
  }
}

bool BaseTreeView::eventFilter(QObject *object, QEvent *event)
{
  if ((event->type() == QEvent::FocusIn)
      || (event->type() == QEvent::MouseButtonPress)) {
    emit activated(this);
  }

  return QWidget::eventFilter(object, event);
}

void BaseTreeView::bindModel(QAbstractItemModel *model)
{
  if (this->model != nullptr) {
    disconnect(this->model, &QAbstractItemModel::rowsInserted, this,
        &BaseTreeView::onModelRowCountChanged);
    disconnect(this->model, &QAbstractItemModel::rowsRemoved, this,
        &BaseTreeView::onModelRowCountChanged);
  }
  if (treeView->selectionModel() != nullptr) {
    disconnect(treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged, this,
        &BaseTreeView::onViewSelectionChanged);
  }

  this->model = model;
  treeView->setModel(model);

  if (model != nullptr) {
    connect(model, &QAbstractItemModel::rowsInserted, this,
        &BaseTreeView::onModelRowCountChanged);
    connect(model, &QAbstractItemModel::rowsRemoved, this,
        &BaseTreeView::onModelRowCountChanged);
  }

  if (treeView->selectionModel() != nullptr) {
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &BaseTreeView::onViewSelectionChanged);
  }

  onUserLevelChanged(ctx.userLevel().getLevel());

  emit availabilityChanged();
  emit selectionChanged(-1);
}

int BaseTreeView::getCurrentRow() const
{
  const QModelIndexList selected = treeView->selectionModel()->selectedRows();
  if (selected.isEmpty()) {
    return -1;
  }
  return selected.first().row();
}

int BaseTreeView::getCurrentColumn() const
{
  const QModelIndexList selected = treeView->selectionModel()->selectedRows();
  if (selected.isEmpty()) {
    return -1;
  }
  return selected.first().column();
}

void BaseTreeView::createView(const QString &title, bool bold)
{
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  if (!title.isEmpty()) {
    header = new QLabel(title, this);
    auto font = header->font();
    font.setBold(bold);
    header->setFont(font);
    layout->addWidget(header);
  }

  setupTreeView();
  layout->addWidget(treeView);
}

void BaseTreeView::setupTreeView()
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

  // default styles often don't distinguish Active/Inactive selection, so
  // an unfocused tree view's selected row still looks "active"; grey it
  // out explicitly using the Disabled palette's highlight colors.
  auto palette = treeView->palette();
  palette.setColor(QPalette::Inactive, QPalette::Highlight,
      palette.color(QPalette::Disabled, QPalette::Highlight));
  palette.setColor(QPalette::Inactive, QPalette::HighlightedText,
      palette.color(QPalette::Disabled, QPalette::HighlightedText));
  treeView->setPalette(palette);

  treeView->setEditTriggers(
      QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

void BaseTreeView::createConnections()
{
  connect(&ctx.userLevel(), &lib::UserLevel::levelChanged, this,
      &BaseTreeView::onUserLevelChanged);
  connect(&ctx.settings(), &Settings::settingsAccepted, this,
      &BaseTreeView::onSettingsChanged);
}

void BaseTreeView::createEventFilter()
{
  installEventFilter(this);

  const QList<QWidget*> children = findChildren<QWidget*>();

  for (QWidget *child : children) {
    child->installEventFilter(this);
  }
}

} // namespace editors
