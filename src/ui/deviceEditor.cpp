// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSplitter>

#include <DockManager.h>

#include "lib/icon.h"
#include "deviceEditor.h"
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

void DeviceEditor::onSelectionChanged(int row)
{
  auto deviceId =
      reinterpret_cast<DeviceTreeView*>(mainView)->getCurrentDeviceId();

  updateHardButtonView(deviceId);
  updateSoftButtonView(deviceId);

  updateActions();

  emit selectionChanged(deviceId);
}

void DeviceEditor::createView()
{
  BaseEditor::createView();

  auto *splitter = new QSplitter(Qt::Vertical, this);

  mainView = new DeviceTreeView(ctx, this);
  splitter->addWidget(mainView);

  auto *buttonSplitter = new QSplitter(Qt::Horizontal, splitter);
  hardButtonView = new DeviceHardButtonTreeView(ctx, this);
  buttonSplitter->addWidget(hardButtonView);
  childViews.append(hardButtonView);
  softButtonView = new DeviceSoftButtonTreeView(ctx, this);
  buttonSplitter->addWidget(softButtonView);
  childViews.append(softButtonView);
  splitter->addWidget(buttonSplitter);

  layout->addWidget(splitter);
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

} // namespace editors
