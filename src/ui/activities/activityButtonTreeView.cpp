// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QTreeView>

#include "ui/delegates/combobox.h"
#include "ui/delegates/uidCombobox.h"
#include "activityButtonTreeView.h"
#include "models/buttonListModel.h"

using namespace std;

namespace editors
{

ActivityHardButtonTreeView::ActivityHardButtonTreeView(Context &ctx,
    QWidget *parent) :
    BaseTreeView(ctx, tr("Hard Buttons"), true, parent)
{
  setupDelegates();
}

ActivityHardButtonTreeView::~ActivityHardButtonTreeView() = default;

void ActivityHardButtonTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);
}

void ActivityHardButtonTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  auto *uidComboBoxDelegate = new delegates::UidComboBox(this);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ActivityHardButtonModel::Column::DEVICE),
      uidComboBoxDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ActivityHardButtonModel::Column::COMMAND),
      comboBoxDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ActivityHardButtonModel::Column::BUTTON),
      comboBoxDelegate);
}

ActivitySoftButtonTreeView::ActivitySoftButtonTreeView(Context &ctx,
    QWidget *parent) :
    BaseTreeView(ctx, tr("Touch Buttons"), true, parent)
{
  setupDelegates();
}

ActivitySoftButtonTreeView::~ActivitySoftButtonTreeView() = default;

void ActivitySoftButtonTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);
}

void ActivitySoftButtonTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  auto *uidComboBoxDelegate = new delegates::UidComboBox(this);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ActivitySoftButtonModel::Column::DEVICE),
      uidComboBoxDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ActivitySoftButtonModel::Column::COMMAND),
      comboBoxDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ActivitySoftButtonModel::Column::ICON),
      comboBoxDelegate);
}

} // namespace editors
