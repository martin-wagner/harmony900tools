// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSplitter>

#include <DockManager.h>

#include "lib/icon.h"
#include "activityEditor.h"
#include "activityTreeView.h"
#include "activityButtonTreeView.h"

using namespace std;

namespace editors
{

ActivityEditor::ActivityEditor(Context &ctx, models::ActivityModel *model,
    QWidget *parent) :
    BaseEditor(ctx, parent)
{
  createView();
  createConnections();
  setModel(model);
}

ActivityEditor::~ActivityEditor() = default;

void ActivityEditor::setModel(models::ActivityModel *model)
{
  mainView->setModel(model);
}

void ActivityEditor::onSelectionChanged(int row)
{
  auto activityId =
      reinterpret_cast<ActivityTreeView*>(mainView)->getCurrentActivityId();

  updateHardButtonView(activityId);
  updateSoftButtonView(activityId);

  updateActions();

  emit selectionChanged(row);
}

void ActivityEditor::createView()
{
  BaseEditor::createView();

  auto *splitter = new QSplitter(Qt::Vertical, this);

  mainView = new ActivityTreeView(ctx, this);
  splitter->addWidget(mainView);
  addHLine(splitter);

  auto *buttonSplitter = new QSplitter(Qt::Horizontal, splitter);
  hardButtonView = new ActivityHardButtonTreeView(ctx, this);
  buttonSplitter->addWidget(hardButtonView);
  childViews.append(hardButtonView);
  softButtonView = new ActivitySoftButtonTreeView(ctx, this);
  buttonSplitter->addWidget(softButtonView);
  childViews.append(softButtonView);
  splitter->addWidget(buttonSplitter);

  layout->addWidget(splitter);
}

void ActivityEditor::updateHardButtonView(uint32_t activityId)
{
  hardButtonView->setModel(nullptr);
  if (hardButtonModel != nullptr) {
    hardButtonModel->deleteLater();
    hardButtonModel = nullptr;
  }
  if (activityId > 0) {
    hardButtonModel = new models::ActivityHardButtonModel(*ctx.config(), activityId,
        this);
    connect(hardButtonModel, &models::ActivityHardButtonModel::writeLog, this,
        &ActivityEditor::writeLog);
    connect(hardButtonModel, &models::ActivityHardButtonModel::writeMsg, this,
        &ActivityEditor::writeMsg);
    hardButtonView->setModel(hardButtonModel);
  }
}

void ActivityEditor::updateSoftButtonView(uint32_t activityId)
{
  softButtonView->setModel(nullptr);
  if (softButtonModel != nullptr) {
    softButtonModel->deleteLater();
    softButtonModel = nullptr;
  }
  if (activityId > 0) {
    softButtonModel = new models::ActivitySoftButtonModel(*ctx.config(), activityId,
        this);
    connect(softButtonModel, &models::ActivitySoftButtonModel::writeLog, this,
        &ActivityEditor::writeLog);
    connect(softButtonModel, &models::ActivitySoftButtonModel::writeMsg, this,
        &ActivityEditor::writeMsg);
    softButtonView->setModel(softButtonModel);
  }
}

} // namespace editors
