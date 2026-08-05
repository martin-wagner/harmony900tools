// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QTreeView>

#include "activityTreeView.h"
#include "models/activityListModel.h"
#include "delegates/combobox.h"

using namespace std;

namespace editors
{

ActivityTreeView::ActivityTreeView(Context &ctx, QWidget *parent) :
    BaseTreeView(ctx, QString(), parent)
{
  setupDelegates();
}

ActivityTreeView::~ActivityTreeView() = default;

void ActivityTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);

  onSettingsChanged();
  onUserLevelChanged(ctx.userLevel().getLevel());
}

uint32_t ActivityTreeView::getCurrentActivityId() const
{
  const int row = getCurrentRow();
  if ((model == nullptr) || (row < 0)) {
    return 0;
  }
  return activityIdForRow(row);
}

uint32_t ActivityTreeView::activityIdForRow(int row) const
{
  auto *activityModel = static_cast<models::ActivityModel *>(model);
  auto idIndex = activityModel->index(row, models::ActivityModel::Column::ID);
  return activityModel->data(idIndex, Qt::DisplayRole).toUInt();
}

void ActivityTreeView::onUserLevelChanged(lib::UserLevel::Level l)
{
  BaseTreeView::onUserLevelChanged(l);

  if (lib::UserLevel::validate(l, lib::UserLevel::Level::Developer)) {
    treeView->showColumn(models::ActivityModel::Column::ID);
  } else {
    treeView->hideColumn(models::ActivityModel::Column::ID);
  }
}

void ActivityTreeView::onSettingsChanged()
{
  BaseTreeView::onSettingsChanged();
}

void ActivityTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  treeView->setItemDelegateForColumn(models::ActivityModel::Column::ACTTYPE,
      comboBoxDelegate);
}

} // namespace editors
