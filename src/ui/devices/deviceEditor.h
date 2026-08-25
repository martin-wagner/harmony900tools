// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "ui/concordConnection.h"
#include "ui/baseEditor.h"
#include "models/buttonListModel.h"
#include "models/deviceListModel.h"
#include "models/protocolIrListModel.h"
#include "models/rawIrListModel.h"
#include "models/statemachineListModel.h"

namespace editors
{

class CommandEditorView;
class StateMachineEditorView;
class DeviceTreeView;
class RawCommandTreeView;
class ProtoCommandTreeView;
class DeviceHardButtonTreeView;
class DeviceSoftButtonTreeView;
class StateMachineTreeView;
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

  signals:
    /** we are ready to process a learned IR command */
    void enableLearnMode(bool start);

  public slots:
    /** add a learned command to the command lists. dropped if none is active */
    void setLearnedCommand(ConcordConnection::LearnedCommandMode m, const binary::TimingStream &t, uint32_t carrier);

  protected slots:
    virtual void onSelectionChanged(int row) override;
    virtual void onImportClicked() override;
    virtual void onExportClicked() override;

  private:
    /** all child views, in display order */
    CommandEditorView *commandEditorView = nullptr;
    ProtoCommandTreeView *protoCommandView = nullptr;
    RawCommandTreeView *rawCommandView = nullptr;
    DeviceHardButtonTreeView *hardButtonView = nullptr;
    DeviceSoftButtonTreeView *softButtonView = nullptr;
    StateMachineEditorView *stateMachineEditorView = nullptr;
    StateMachineTreeView *stateMachineView = nullptr;
    //models
    models::ProtocolIrModel *protoCommandModel = nullptr;
    models::RawIrModel *rawCommandModel = nullptr;
    models::DeviceHardButtonModel *hardButtonModel = nullptr;
    models::DeviceSoftButtonModel *softButtonModel = nullptr;
    models::StateMachineModel *stateMachineModel = nullptr;

    virtual void createView() override;
    virtual void createConnections() override;

  private:
    void updateLearnMode(int row);

    //create model for deviceId
    void updateCommandEditorView(uint32_t deviceId);
    void updateHardButtonView(uint32_t deviceId);
    void updateSoftButtonView(uint32_t deviceId);
    void updateStateMachineView(uint32_t deviceId);
};

} // namespace editors
