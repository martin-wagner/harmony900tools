// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "baseTreeView.h"

namespace models
{
class ActivityHardButtonModel;
}

namespace editors
{

/**
 * @brief Flat hard-button list tree view for the currently selected activity.
 */
class ActivityHardButtonTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit ActivityHardButtonTreeView(Context &ctx, QWidget *parent = nullptr);
    ~ActivityHardButtonTreeView() override;

    void setModel(QAbstractItemModel *model) override;

  private:
    void setupDelegates();
};


/**
 * @brief Flat touch-button list tree view for the currently selected activity.
 */
class ActivitySoftButtonTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit ActivitySoftButtonTreeView(Context &ctx, QWidget *parent = nullptr);
    ~ActivitySoftButtonTreeView() override;

    void setModel(QAbstractItemModel *model) override;

  private:
    void setupDelegates();
};

} // namespace editors
