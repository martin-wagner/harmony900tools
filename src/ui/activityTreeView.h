// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "baseTreeView.h"

namespace models
{
class ActivityModel;
}

namespace editors
{

/**
 * @brief Flat activity list tree view (master).
 */
class ActivityTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit ActivityTreeView(Context &ctx, QWidget *parent = nullptr);
    ~ActivityTreeView() override;

    /** set new model. nullptr = remove */
    void setModel(QAbstractItemModel *model) override;

    virtual bool supportsMoveOperation() const override { return true; };

    /** currently selected activity id, or 0 if nothing is selected */
    uint32_t getCurrentActivityId() const;

  protected:
    uint32_t activityIdForRow(int row) const;

  protected slots:
    virtual void onUserLevelChanged(lib::UserLevel::Level l) override;
    virtual void onSettingsChanged() override;

  private:
    void setupDelegates();
};

} // namespace editors
