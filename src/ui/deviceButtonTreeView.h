// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "baseTreeView.h"

namespace models
{
class DeviceHardButtonModel;
}

namespace editors
{

/**
 * @brief Flat hard-button list tree view for the currently selected device.
 */
class DeviceHardButtonTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit DeviceHardButtonTreeView(Context &ctx, QWidget *parent = nullptr);
    ~DeviceHardButtonTreeView() override;

    void setModel(QAbstractItemModel *model) override;

  private:
    void setupDelegates();
};

} // namespace editors
