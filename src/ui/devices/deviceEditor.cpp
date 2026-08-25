// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSplitter>

#include <DockManager.h>

#include "lib/icon.h"
#include "document/files/sharing.h"
#include "deviceEditor.h"
#include "ir/commandEditorView.h"
#include "statemachine/stateMachineEditorView.h"
#include "deviceTreeView.h"
#include "deviceButtonTreeView.h"

using namespace std;

namespace editors
{

DeviceEditor::DeviceEditor(Context &ctx, models::DeviceModel *model,
    QWidget *parent) :
    BaseEditor(ctx, parent)
{
  createView();
  createConnections();
  setModel(model);
}

DeviceEditor::~DeviceEditor() = default;

void DeviceEditor::setModel(models::DeviceModel *model)
{
  mainView->setModel(model);
}

void editors::DeviceEditor::setLearnedCommand(
    ConcordConnection::LearnedCommandMode m, const binary::TimingStream &t,
    uint32_t carrier)
{
  if (lastActiveView == protoCommandView) {
    protoCommandView->setLearnedCommand(m, t, carrier);
  } else if (lastActiveView == rawCommandView) {
    rawCommandView->setLearnedCommand(m, t, carrier);
  }
}

void DeviceEditor::onSelectionChanged(int row)
{
  auto deviceId =
      reinterpret_cast<DeviceTreeView*>(mainView)->getCurrentDeviceId();

  updateCommandEditorView(deviceId);
  updateHardButtonView(deviceId);
  updateSoftButtonView(deviceId);
  updateStateMachineView(deviceId);

  updateActions();

  emit selectionChanged(row);
}

void DeviceEditor::onImportClicked()
{
  auto reader = document::files::DeviceStorage();
  reader.import(*ctx.config());
}

void DeviceEditor::onExportClicked()
{
  auto deviceId =
      reinterpret_cast<DeviceTreeView*>(mainView)->getCurrentDeviceId();

  auto writer = document::files::DeviceStorage();
  writer.write(ctx.config()->data(), deviceId);
}

void DeviceEditor::createView()
{
  BaseEditor::createView();

  actionExport->setVisible(true);
  actionImport->setVisible(true);

  auto *splitter = new QSplitter(Qt::Vertical, this);

  mainView = new DeviceTreeView(ctx, this);
  splitter->addWidget(mainView);
  addHLine(splitter);

  commandEditorView = new CommandEditorView(ctx, this);
  splitter->addWidget(commandEditorView);
  protoCommandView = new ProtoCommandTreeView(ctx, this);
  rawCommandView = new RawCommandTreeView(ctx, this);
  commandEditorView->addTreeViews(protoCommandView, rawCommandView);
  childViews.append(protoCommandView);
  childViews.append(rawCommandView);
  addHLine(splitter);

  auto *buttonSplitter = new QSplitter(Qt::Horizontal, splitter);
  hardButtonView = new DeviceHardButtonTreeView(ctx, this);
  buttonSplitter->addWidget(hardButtonView);
  childViews.append(hardButtonView);
  softButtonView = new DeviceSoftButtonTreeView(ctx, this);
  buttonSplitter->addWidget(softButtonView);
  childViews.append(softButtonView);
  splitter->addWidget(buttonSplitter);

  stateMachineEditorView = new StateMachineEditorView(ctx, this);
  splitter->addWidget(stateMachineEditorView);
  stateMachineView = new StateMachineTreeView(ctx, this);
  stateMachineEditorView->addTreeView(stateMachineView);
  childViews.append(stateMachineView);

  layout->addWidget(splitter);
}

void DeviceEditor::createConnections()
{
  BaseEditor::createConnections();

  connect(protoCommandView, &ProtoCommandTreeView::selectionChanged, this,
      &DeviceEditor::updateLearnMode);
  connect(rawCommandView, &RawCommandTreeView::selectionChanged, this,
      &DeviceEditor::updateLearnMode);

  connect(commandEditorView, &CommandEditorView::writeLog, this,
      &DeviceEditor::writeLog);
  connect(commandEditorView, &CommandEditorView::writeMsg, this,
      &DeviceEditor::writeMsg);

  connect(stateMachineEditorView, &StateMachineEditorView::writeLog, this,
      &DeviceEditor::writeLog);
  connect(stateMachineEditorView, &StateMachineEditorView::writeMsg, this,
      &DeviceEditor::writeMsg);
}

void DeviceEditor::updateLearnMode(int row)
{
  if (row >= 0) {
    emit enableLearnMode(true);
  } else {
    emit enableLearnMode(false);
  }
}

void DeviceEditor::updateCommandEditorView(uint32_t deviceId)
{
  commandEditorView->setData(0, nullptr, nullptr);
  protoCommandView->setModel(nullptr);
  if (protoCommandModel != nullptr) {
    protoCommandModel->deleteLater();
    protoCommandModel = nullptr;
  }
  rawCommandView->setModel(nullptr);
  if (rawCommandModel != nullptr) {
    rawCommandModel->deleteLater();
    rawCommandModel = nullptr;
  }

  if (deviceId > 0) {
    protoCommandModel = new models::ProtocolIrModel(*ctx.config(), deviceId,
        this);
    connect(protoCommandModel, &models::ProtocolIrModel::writeLog, this,
        &DeviceEditor::writeLog);
    connect(protoCommandModel, &models::ProtocolIrModel::writeMsg, this,
        &DeviceEditor::writeMsg);
    protoCommandView->setModel(protoCommandModel);

    rawCommandModel = new models::RawIrModel(*ctx.config(), deviceId, this);
    connect(rawCommandModel, &models::RawIrModel::writeLog, this,
        &DeviceEditor::writeLog);
    connect(rawCommandModel, &models::RawIrModel::writeMsg, this,
        &DeviceEditor::writeMsg);
    rawCommandView->setModel(rawCommandModel);

    commandEditorView->setData(deviceId, protoCommandModel, rawCommandModel);
  }
}

void DeviceEditor::updateHardButtonView(uint32_t deviceId)
{
  hardButtonView->setModel(nullptr);
  if (hardButtonModel != nullptr) {
    hardButtonModel->deleteLater();
    hardButtonModel = nullptr;
  }
  if (deviceId > 0) {
    hardButtonModel = new models::DeviceHardButtonModel(*ctx.config(), deviceId,
        this);
    connect(hardButtonModel, &models::DeviceHardButtonModel::writeLog, this,
        &DeviceEditor::writeLog);
    connect(hardButtonModel, &models::DeviceHardButtonModel::writeMsg, this,
        &DeviceEditor::writeMsg);
    hardButtonView->setModel(hardButtonModel);
  }
}

void DeviceEditor::updateSoftButtonView(uint32_t deviceId)
{
  softButtonView->setModel(nullptr);
  if (softButtonModel != nullptr) {
    softButtonModel->deleteLater();
    softButtonModel = nullptr;
  }
  if (deviceId > 0) {
    softButtonModel = new models::DeviceSoftButtonModel(*ctx.config(), deviceId,
        this);
    connect(softButtonModel, &models::DeviceSoftButtonModel::writeLog, this,
        &DeviceEditor::writeLog);
    connect(softButtonModel, &models::DeviceSoftButtonModel::writeMsg, this,
        &DeviceEditor::writeMsg);
    softButtonView->setModel(softButtonModel);
  }
}

void DeviceEditor::updateStateMachineView(uint32_t deviceId)
{
  stateMachineEditorView->setData(0, nullptr);
  stateMachineView->setModel(nullptr);
  if (stateMachineModel != nullptr) {
    stateMachineModel->deleteLater();
    stateMachineModel = nullptr;
  }

  if (deviceId > 0) {
    stateMachineModel = new models::StateMachineModel(*ctx.config(), deviceId,
        this);
    connect(stateMachineModel, &models::StateMachineModel::writeLog, this,
        &DeviceEditor::writeLog);
    connect(stateMachineModel, &models::StateMachineModel::writeMsg, this,
        &DeviceEditor::writeMsg);
    stateMachineView->setModel(stateMachineModel);

    stateMachineEditorView->setData(deviceId, stateMachineModel);
  }
}

} // namespace editors
