// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

#include "deviceActionEditor.h"
#include "actionRowWidget.h"

using namespace document::data::item;

namespace editors
{

DeviceActionEditor::DeviceActionEditor(QWidget *parent) :
    QWidget(parent)
{
  buildUi();
}

void DeviceActionEditor::setTitle(const QString &title)
{
  titleLabel->setText(title);
}

void DeviceActionEditor::setDeviceAction(const DeviceAction &deviceAction)
{
  for (ActionRowWidget *row : rows) {
    row->deleteLater();
  }
  rows.clear();
//
//  for (const Action &action : deviceAction) {
//    addRow(action);
//  }
//
//  if (rows.isEmpty()) {
//    addRow(Action { });
//  }

  refreshDragHandles();
}

DeviceAction DeviceActionEditor::getDeviceAction() const
{
  DeviceAction deviceAction;
//
//  for (const ActionRowWidget *row : rows) {
//    deviceAction.push_back(row->getAction());
//  }

  return deviceAction;
}

void DeviceActionEditor::onAddStepClicked()
{
  //addRow(Action { });
  refreshDragHandles();
  emit changed();
}

void DeviceActionEditor::onRowRemoveRequested()
{
  ActionRowWidget *row = qobject_cast<ActionRowWidget*>(sender());
  if (row == nullptr) {
    return;
  }

  //keep at least one row -- an empty DeviceAction slot reads as "nothing
  //configured yet", which the caller (discrete/relative editor) treats
  //differently from "one no-op step"
  if (rows.size() <= 1) {
    return;
  }

  rows.removeOne(row);
  rowsLayout->removeWidget(row);
  row->deleteLater();

  refreshDragHandles();
  emit changed();
}

void DeviceActionEditor::buildUi()
{
  titleLabel = new QLabel(this);
  titleLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

  rowsLayout = new QVBoxLayout();
  rowsLayout->setSpacing(4);

  addStepButton = new QPushButton(tr("Add step"), this);
  connect(addStepButton, &QPushButton::clicked, this,
      &DeviceActionEditor::onAddStepClicked);

  QLabel *hintLabel = new QLabel(tr("Runs top to bottom."), this);
  hintLabel->setEnabled(false);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addWidget(titleLabel);
  rootLayout->addWidget(hintLabel);
  rootLayout->addLayout(rowsLayout);
  rootLayout->addWidget(addStepButton);
  rootLayout->addStretch(1);
}

void DeviceActionEditor::addRow(const DeviceAction &action)
{
  ActionRowWidget *row = new ActionRowWidget(this);
  row->setAction(action);

  connect(row, &ActionRowWidget::removeRequested, this,
      &DeviceActionEditor::onRowRemoveRequested);
  connect(row, &ActionRowWidget::changed, this, &DeviceActionEditor::changed);

  rowsLayout->addWidget(row);
  rows.append(row);
}

void DeviceActionEditor::refreshDragHandles()
{
  //dragging only makes sense with more than one row
  const bool showHandles = rows.size() > 1;
  for (ActionRowWidget *row : rows) {
    row->setDragHandleVisible(showHandles);
  }
}

}
