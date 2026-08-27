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
#include "sequenceItemWidget.h"

using namespace std;
using namespace document::data::item;
using namespace document::data;

namespace editors
{

DeviceActionEditor::DeviceActionEditor(Context &ctx, uint32_t devicePos,
    uint32_t smPos, QWidget *parent) :
    QWidget(parent), ctx(ctx), devicePos(devicePos), smPos(smPos)
{
  createView();
  createConnections();
}

optional<DeviceAction> DeviceActionEditor::openEditor(Context &ctx,
    uint32_t devicePos, uint32_t smPos, const DeviceAction &deviceAction,
    ActionType type, const QString &title, QWidget *parent)
{
  bool dataChanged = false;

  QDialog dialog(parent);
  dialog.setWindowTitle(tr("Edit action"));

  auto editor = new DeviceActionEditor(ctx, devicePos, smPos, &dialog);
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
  int i;

  //prefer current value
  if (deviceAction.actionType.get().getValue() != ActionType::Unknown) {
    this->typeBox->setCurrentText(deviceAction.actionType.get().getQString());
  } else {
    this->typeBox->setCurrentText(Enum<ActionType>::toQString(type));
  }
  repeatBox->setChecked(deviceAction.repeatWillNotHarm.get());

  for (SequenceItemWidget *row : rows) {
    rowsLayout->removeWidget(row);
    row->deleteLater();
  }
  rows.clear();

  for (i = 0; i < deviceAction.sequence.size(); i++) {
    addRow(deviceAction.sequence[i]);
  }
}

DeviceAction DeviceActionEditor::getDeviceAction() const
{
  DeviceAction deviceAction;

  deviceAction.actionType.set(
      Enum<ActionType>(typeBox->currentText()).getValue());
  deviceAction.repeatWillNotHarm.set(repeatBox->isChecked());

  for (const SequenceItemWidget *row : rows) {
    deviceAction.sequence.push_back(row->getSequenceItem());
  }

  return deviceAction;
}

void DeviceActionEditor::onAddStepClicked()
{
  addRow(SequenceItem());
  emit changed();
}

void DeviceActionEditor::onRowRemoveRequested()
{
  SequenceItemWidget *row = qobject_cast<SequenceItemWidget*>(sender());
  if (row == nullptr) {
    return;
  }

  rows.removeOne(row);
  rowsLayout->removeWidget(row);
  row->deleteLater();

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

void DeviceActionEditor::addRow(const SequenceItem &sequenceItem)
{
  SequenceItemWidget *row = new SequenceItemWidget(ctx, devicePos, smPos,
      SequenceItemWidget::ParentType::DEVICE, this);
  row->setSequenceItem(sequenceItem);

  connect(row, &SequenceItemWidget::removeRequested, this,
      &DeviceActionEditor::onRowRemoveRequested);
  connect(row, &SequenceItemWidget::changed, this,
      &DeviceActionEditor::changed);

  rowsLayout->addWidget(row);
  rows.append(row);
}

}
