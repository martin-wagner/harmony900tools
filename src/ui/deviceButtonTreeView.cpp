// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QTreeView>

#include "deviceButtonTreeView.h"
#include "models/buttonListModel.h"
#include "delegates/combobox.h"

using namespace std;

namespace editors
{

DeviceHardButtonTreeView::DeviceHardButtonTreeView(Context &ctx,
    QWidget *parent) :
    BaseTreeView(ctx, parent)
{
  setupDelegates();
}

DeviceHardButtonTreeView::~DeviceHardButtonTreeView() = default;

void DeviceHardButtonTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);
}

void DeviceHardButtonTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::DeviceHardButtonModel::Column::COMMAND),
      comboBoxDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::DeviceHardButtonModel::Column::BUTTON),
      comboBoxDelegate);
}

} // namespace editors
