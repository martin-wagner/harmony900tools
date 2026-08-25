// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "ui/baseTreeView.h"

namespace models
{
class DeviceModel;
}

namespace editors
{

/**
 * @brief Flat device list tree view (master).
 */
class DeviceTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit DeviceTreeView(Context &ctx, QWidget *parent = nullptr);
    ~DeviceTreeView() override;

    /** set new model. nullptr = remove */
    void setModel(QAbstractItemModel *model) override;

    /** currently selected device id, or 0 if nothing is selected */
    uint32_t getCurrentDeviceId() const;

  protected:
    uint32_t deviceIdForRow(int row) const;

  protected slots:
    virtual void onUserLevelChanged(lib::UserLevel::Level l) override;
    virtual void onSettingsChanged() override;

  private:
    void setupDelegates();
};

} // namespace editors
