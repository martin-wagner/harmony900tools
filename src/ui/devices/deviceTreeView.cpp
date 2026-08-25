// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QTreeView>

#include "deviceTreeView.h"
#include "models/deviceListModel.h"
#include "ui/delegates/combobox.h"

using namespace std;

namespace editors
{

DeviceTreeView::DeviceTreeView(Context &ctx, QWidget *parent) :
    BaseTreeView(ctx, QString(), parent)
{
  setupDelegates();
}

DeviceTreeView::~DeviceTreeView() = default;

void DeviceTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);

  onSettingsChanged();
  onUserLevelChanged(ctx.userLevel().getLevel());
}

uint32_t DeviceTreeView::getCurrentDeviceId() const
{
  const int row = getCurrentRow();
  if ((model == nullptr) || (row < 0)) {
    return 0;
  }
  return deviceIdForRow(row);
}

uint32_t DeviceTreeView::deviceIdForRow(int row) const
{
  auto *deviceModel = static_cast<models::DeviceModel *>(model);
  auto idIndex = deviceModel->index(row, models::DeviceModel::Column::ID);
  return deviceModel->data(idIndex, Qt::DisplayRole).toUInt();
}

void DeviceTreeView::onUserLevelChanged(lib::UserLevel::Level l)
{
  BaseTreeView::onUserLevelChanged(l);

  if (lib::UserLevel::validate(l, lib::UserLevel::Level::Developer)) {
    treeView->showColumn(models::DeviceModel::Column::ID);
  } else {
    treeView->hideColumn(models::DeviceModel::Column::ID);
  }
}

void DeviceTreeView::onSettingsChanged()
{
  BaseTreeView::onSettingsChanged();
}

void DeviceTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  treeView->setItemDelegateForColumn(models::DeviceModel::Column::DEVTYPE,
      comboBoxDelegate);
}

} // namespace editors
