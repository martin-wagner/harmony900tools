// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QDialog>
#include <QDialogButtonBox>

#include "deviceActionEditor.h"
#include "actionRowWidget.h"

using namespace std;
using namespace document::data::item;
using namespace document::data;

namespace editors
{

DeviceActionEditor::DeviceActionEditor(QWidget *parent) :
    QWidget(parent)
{
  createView();
  createConnections();
}

optional<DeviceAction> DeviceActionEditor::openEditor(
    const DeviceAction &deviceAction, ActionType type, const QString &title,
    QWidget *parent)
{
  bool dataChanged = false;

  QDialog dialog(parent);
  dialog.setWindowTitle(tr("Edit action"));

  auto editor = new DeviceActionEditor(&dialog);
  editor->setTitle(title);
  editor->setDeviceAction(deviceAction, type);
  connect(editor, &DeviceActionEditor::changed, [&dataChanged]() {
    dataChanged = true;
  });

  auto *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  auto *layout = new QVBoxLayout(&dialog);
  layout->addWidget(editor);
  layout->addWidget(buttonBox);

  if ((dialog.exec() == QDialog::Accepted) && dataChanged) {
    return editor->getDeviceAction();
  }
  return nullopt;
}

void DeviceActionEditor::setTitle(const QString &title)
{
  titleLabel->setText(title);
}

void DeviceActionEditor::setDeviceAction(const DeviceAction &deviceAction,
    ActionType type)
{
  this->deviceAction = deviceAction;

  //prefer current value
  if (deviceAction.actionType.get().getValue() != ActionType::Unknown) {
    this->typeBox->setCurrentText(deviceAction.actionType.get().getQString());
  } else {
    this->typeBox->setCurrentText(Enum<ActionType>::toQString(type));
  }
  repeatBox->setChecked(deviceAction.repeatWillNotHarm.get());

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
  auto a = deviceAction; //copy trough

  a.actionType.set(Enum<ActionType>(typeBox->currentText()).getValue());
  a.repeatWillNotHarm.set(repeatBox->isChecked());

  //todo properties

  for (const ActionRowWidget *row : rows) {
    //deviceAction.sequence.push_back(row->getAction());
  }

  return a;
}

void DeviceActionEditor::onAddStepClicked()
{
  addRow(DeviceAction());
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

void DeviceActionEditor::createView()
{
  titleLabel = new QLabel(this);
  titleLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

  rowsLayout = new QVBoxLayout();
  rowsLayout->setSpacing(4);

  QHBoxLayout *typeLayout = new QHBoxLayout();
  typeBox = new QComboBox(this);
  typeBox->setToolTip(tr("What the remote will do internally"));
  typeBox->addItems(Enum<ActionType>::toQStringList());
  typeBox->setEnabled(false);
  QLabel *typeLabel = new QLabel(tr("Action Type"), this);
  typeLabel->setToolTip(typeBox->toolTip());
  typeLayout->addWidget(typeLabel);
  typeLayout->addWidget(typeBox);
  typeLayout->addStretch(1);

  QHBoxLayout *repeatLayout = new QHBoxLayout();
  repeatBox = new QCheckBox(this);
  repeatBox->setToolTip(
      tr("This command sequence can be repeated any number of times "
          "without changing the behaviour. Keep off if not sure."));
  QLabel *repeatLabel = new QLabel(tr("Can repeat"), this);
  repeatLabel->setToolTip(repeatBox->toolTip());
  repeatLayout->addWidget(repeatLabel);
  repeatLayout->addWidget(repeatBox);
  repeatLayout->addStretch(1);

  QLabel *hintLabel = new QLabel(tr("Runs top to bottom."), this);
  hintLabel->setEnabled(false);

  addStepButton = new QPushButton(tr("Add step"), this);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addWidget(titleLabel);
  rootLayout->addLayout(typeLayout);
  rootLayout->addLayout(repeatLayout);
  rootLayout->addWidget(hintLabel);
  rootLayout->addLayout(rowsLayout);
  rootLayout->addWidget(addStepButton);
  rootLayout->addStretch(1);
}

void DeviceActionEditor::createConnections()
{
  connect(repeatBox, &QCheckBox::toggled, this, &DeviceActionEditor::changed);
  connect(addStepButton, &QPushButton::clicked, this,
      &DeviceActionEditor::onAddStepClicked);
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
