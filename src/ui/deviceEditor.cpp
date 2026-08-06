// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSplitter>

#include <DockManager.h>

#include "lib/icon.h"
#include "deviceEditor.h"
#include "commandEditorView.h"
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

  updateCommandEditorView(deviceId);
  updateHardButtonView(deviceId);
  updateSoftButtonView(deviceId);

  updateActions();

  emit selectionChanged(row);
}

void DeviceEditor::createView()
{
  BaseEditor::createView();

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

  layout->addWidget(splitter);
}

void editors::DeviceEditor::createConnections()
{
  BaseEditor::createConnections();

  connect(commandEditorView, &CommandEditorView::writeLog, this,
      &DeviceEditor::writeLog);
  connect(commandEditorView, &CommandEditorView::writeMsg, this,
      &DeviceEditor::writeMsg);
}

void editors::DeviceEditor::updateCommandEditorView(uint32_t deviceId)
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
//    commandEditorView = new CommandEditorView(*ctx.config(), deviceId,
//        this);
//    connect(commandEditorView, &models::DeviceHardButtonModel::writeLog, this,
//        &DeviceEditor::writeLog);
//    connect(commandEditorView, &models::DeviceHardButtonModel::writeMsg, this,
//        &DeviceEditor::writeMsg); todo
    protoCommandModel = new QStandardItemModel(4, 4);
    for (int row = 0; row < protoCommandModel->rowCount(); ++row) {
      for (int column = 0; column < protoCommandModel->columnCount();
          ++column) {
        QStandardItem *item = new QStandardItem(
            QString("row %0, column %1").arg(row).arg(column));
        protoCommandModel->setItem(row, column, item);
      }
    }
    protoCommandView->setModel(protoCommandModel); //todo richtiges modell eintragen

    rawCommandModel = new QStandardItemModel(4, 4);
    for (int row = 0; row < rawCommandModel->rowCount(); ++row) {
      for (int column = 0; column < rawCommandModel->columnCount(); ++column) {
        QStandardItem *item = new QStandardItem(
            QString("row %0, column %1").arg(row).arg(column));
        rawCommandModel->setItem(row, column, item);
      }
    }
    rawCommandView->setModel(rawCommandModel); //todo richtiges modell eintragen

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

} // namespace editors
