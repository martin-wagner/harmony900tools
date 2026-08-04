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
    ctx(ctx)
{
  createView();
  createConnections();
  setModel(model);
}

DeviceEditor::~DeviceEditor() = default;

void DeviceEditor::setModel(models::DeviceModel *model)
{
  deviceView->setModel(model);
}

void DeviceEditor::onAddClicked()
{
  if (lastActiveView == nullptr) {
    return;
  }
  lastActiveView->addRow();
}

void DeviceEditor::onRemoveClicked()
{
  if (lastActiveView == nullptr) {
    return;
  }
  lastActiveView->removeRow();
}

void DeviceEditor::onAvailabilityChanged()
{
  updateActions();
}

void DeviceEditor::onDeviceSelectionChanged(int row)
{
  auto deviceId = deviceView->getCurrentDeviceId();

  updateHardButtonView(deviceId);
  updateSoftButtonView(deviceId);

  updateActions();

  emit selectionChanged(deviceId);
}

void DeviceEditor::createView()
{
  //fixme use ads for this
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  setupToolbar();
  layout->addWidget(toolbar);

  auto *splitter = new QSplitter(Qt::Vertical, this);

  deviceView = new DeviceTreeView(ctx, this);
  splitter->addWidget(deviceView);

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

void DeviceEditor::setupToolbar()
{
  toolbar = new QToolBar(this);
  //toolbar->setIconSize(QSize(16, 16));
  toolbar->setFloatable(false);
  toolbar->setMovable(false);

  actionAdd = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/list-add.png",
          "list-add"), tr("Add"));
  actionAdd->setToolTip(tr("Add a new entry to the selected list. Click into\n"
      "the empty space to select for adding the first item."));

  actionRemove = toolbar->addAction(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
          "edit-delete"), tr("Remove"));
  actionRemove->setToolTip(tr("Remove the selected entry"));
  actionRemove->setShortcut(QKeySequence::Delete);
  actionRemove->setShortcutContext(Qt::ApplicationShortcut);

  toolbar->addSeparator();
}

void DeviceEditor::createConnections()
{
  connect(actionAdd, &QAction::triggered, this, &DeviceEditor::onAddClicked);
  connect(actionRemove, &QAction::triggered, this,
      &DeviceEditor::onRemoveClicked);

  connect(deviceView, &DeviceTreeView::availabilityChanged, this,
      &DeviceEditor::onAvailabilityChanged);
  connect(deviceView, &DeviceTreeView::selectionChanged, this,
      &DeviceEditor::onDeviceSelectionChanged);
  connect(deviceView, &DeviceTreeView::activated, this,
      [this](BaseTreeView *view) {
        lastActiveView = view;
      });

  for (auto *childView : childViews) {
    connect(childView, &BaseTreeView::availabilityChanged, this,
        &DeviceEditor::onAvailabilityChanged);
    connect(childView, &BaseTreeView::activated, this,
        [this](BaseTreeView *view) {
          lastActiveView = view;
        });
  }
}

void DeviceEditor::updateActions()
{
  if (lastActiveView == nullptr) {
    actionAdd->setEnabled(false);
    actionRemove->setEnabled(false);
    return;
  }
  actionAdd->setEnabled(true);
  actionRemove->setEnabled(lastActiveView->canRemove());
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
