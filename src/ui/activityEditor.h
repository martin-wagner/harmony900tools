// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "models/buttonListModel.h"
#include "models/activityListModel.h"
#include "baseEditor.h"

namespace editors
{

class ActivityTreeView;
class ActivityHardButtonTreeView;
class ActivitySoftButtonTreeView;

/**
 * @brief Activity editor
 *
 * For editing remote controls
 */
class ActivityEditor: public BaseEditor
{
  Q_OBJECT

  public:
    explicit ActivityEditor(Context &ctx, models::ActivityModel *model, QWidget *parent = nullptr);
    ~ActivityEditor() override;

    /** set new activity model. nullptr = remove */
    void setModel(models::ActivityModel *model);

  protected slots:
    virtual void onSelectionChanged(int row) override;

  private:
    /** all child views, in display order */
    ActivityHardButtonTreeView *hardButtonView = nullptr;
    //ActivitySoftButtonTreeView *softButtonView = nullptr;
    //models
    models::ActivityHardButtonModel *hardButtonModel = nullptr;
    //models::ActivitySoftButtonModel *softButtonModel = nullptr;

    virtual void createView() override;

    //create model for activityId
    void updateHardButtonView(uint32_t activityId);
    void updateSoftButtonView(uint32_t activityId);
};

} // namespace editors
