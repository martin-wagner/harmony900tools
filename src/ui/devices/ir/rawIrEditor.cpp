// SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QVBoxLayout>

#include "lib/icon.h"
#include "rawIrEditor.h"

using namespace std;
using namespace binary::ssIr;

namespace editors
{

RawIrEditor::RawIrEditor(const SerialStreamIr &stream, QWidget *parent) :
    QDialog(parent)
{
  setWindowTitle(tr("Edit IR Data"));

  table = new QTableWidget(0, 2, this);
  table->setHorizontalHeaderLabels( { tr("Mark (µs)"), tr("Pause (µs)") });
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->setVisible(false);

  durationLabel = new QLabel(this);

  for (const auto &block : stream.accessStream().timings()) {
    addRow(table->rowCount(), block.mark_us, block.pause_us);
  }

  connect(table, &QTableWidget::itemChanged, this, &RawIrEditor::itemChanged);

  auto *addButton = new QPushButton(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/list-add.png",
          "list-add"), tr("Add row"), this);
  auto *removeButton = new QPushButton(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
          "edit-delete"), tr("Remove row"), this);
  connect(addButton, &QPushButton::clicked, this, &RawIrEditor::addRowClicked);
  connect(removeButton, &QPushButton::clicked, this,
      &RawIrEditor::removeRowClicked);

  auto *rowButtonLayout = new QHBoxLayout();
  rowButtonLayout->addWidget(addButton);
  rowButtonLayout->addWidget(removeButton);
  rowButtonLayout->addStretch();

  clockBox = new QDoubleSpinBox(this);
  clockBox->setRange(31500.0, 250000.0);
  clockBox->setSingleStep(100.0);
  clockBox->setSuffix(tr(" Hz"));
  clockBox->setValue(stream.getClock());

  auto *formLayout = new QFormLayout();
  formLayout->addRow(tr("Clock:"), clockBox);

  auto *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &RawIrEditor::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &RawIrEditor::reject);

  auto *layout = new QVBoxLayout(this);
  layout->addLayout(formLayout);
  layout->addWidget(table);
  layout->addWidget(durationLabel);
  layout->addLayout(rowButtonLayout);
  layout->addWidget(buttonBox);

  resize(400, 500);

  updateDuration();
}

RawIrEditor::~RawIrEditor() = default;

SerialStreamIr RawIrEditor::getStream() const
{
  vector<uint16_t> raw;

  for (int row = 0; row < table->rowCount(); row++) {
    auto *markItem = table->item(row, 0);
    auto *pauseItem = table->item(row, 1);
    if ((markItem == nullptr) || (pauseItem == nullptr)) {
      continue;
    }
    raw.push_back(static_cast<uint16_t>(markItem->text().toUInt()));
    raw.push_back(static_cast<uint16_t>(pauseItem->text().toUInt()));
  }

  auto timingStream = binary::TimingStream::fromMarkPause(raw);
  return SerialStreamIr(timingStream, clockBox->value());
}

void RawIrEditor::addRow(int row, uint16_t mark_us, uint16_t pause_us)
{
  mark_us = min(mark_us, MAX_VALUE);
  pause_us = min(pause_us, MAX_VALUE);

  table->insertRow(row);
  table->setItem(row, 0, new QTableWidgetItem(QString::number(mark_us)));
  table->setItem(row, 1, new QTableWidgetItem(QString::number(pause_us)));

  updateDuration();
}

void RawIrEditor::addRowClicked()
{
  //insert after the current selection, or at the end if nothing is selected
  auto row = table->currentRow();
  if (row < 0) {
    row = table->rowCount();
  } else {
    row = row + 1;
  }
  addRow(row, 0, 1);
}

void RawIrEditor::removeRowClicked()
{
  auto row = table->currentRow();
  if (row < 0) {
    return;
  }
  table->removeRow(row);
  updateDuration();
}

void RawIrEditor::itemChanged(QTableWidgetItem *item)
{
  if (item == nullptr) {
    return;
  }

  bool ok = false;
  auto value = item->text().toLong(&ok);
  if (!ok) {
    value = 0;
  }
  value = clamp<long>(value, 0, MAX_VALUE);

  //avoid re-entering itemChanged while normalising the text
  const QSignalBlocker blocker(table);
  item->setText(QString::number(value));

  updateDuration();
}

void RawIrEditor::updateDuration()
{
  uint64_t total_us = 0;

  for (int row = 0; row < table->rowCount(); row++) {
    auto *markItem = table->item(row, 0);
    auto *pauseItem = table->item(row, 1);
    if ((markItem == nullptr) || (pauseItem == nullptr)) {
      continue;
    }
    total_us += markItem->text().toULong();
    total_us += pauseItem->text().toULong();
  }

  durationLabel->setText(tr("Duration: %1 ms").arg(total_us / 1000));
}

}
