// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStandardItemModel> //todo remove

#include "models/buttonListModel.h"
#include "models/deviceListModel.h"
#include "models/rawIrListModel.h"
#include "baseEditor.h"

namespace editors
{

class DeviceTreeView;
class CommandEditorView;
class RawCommandTreeView;
class ProtoCommandTreeView;
class DeviceHardButtonTreeView;
class DeviceSoftButtonTreeView;

/**
 * @brief Device editor
 *
 * For editing remote controls
 */
class DeviceEditor: public BaseEditor
{
  Q_OBJECT

  public:
    explicit DeviceEditor(Context &ctx, models::DeviceModel *model, QWidget *parent = nullptr);
    ~DeviceEditor() override;

    /** set new device model. nullptr = remove */
    void setModel(models::DeviceModel *model);

  protected slots:
    virtual void onSelectionChanged(int row) override;

  private:
    /** all child views, in display order */
    CommandEditorView *commandEditorView = nullptr;
    ProtoCommandTreeView *protoCommandView = nullptr;
    RawCommandTreeView *rawCommandView = nullptr;
    DeviceHardButtonTreeView *hardButtonView = nullptr;
    DeviceSoftButtonTreeView *softButtonView = nullptr;
    //models
    QStandardItemModel*protoCommandModel = nullptr;
    models::RawIrModel *rawCommandModel = nullptr;
    models::DeviceHardButtonModel *hardButtonModel = nullptr;
    models::DeviceSoftButtonModel *softButtonModel = nullptr;

    virtual void createView() override;
    virtual void createConnections() override;

    //create model for deviceId
    void updateCommandEditorView(uint32_t deviceId);
    void updateHardButtonView(uint32_t deviceId);
    void updateSoftButtonView(uint32_t deviceId);
};

} // namespace editors
