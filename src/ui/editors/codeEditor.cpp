// SPDX-License-Identifier: LGPL-2.1-or-later

#include "codeEditor.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

using namespace binary::irProto;
using namespace document::data;

namespace editors
{

namespace
{
constexpr double SYSCLOCK_HZ = 18000000.0;

//RC5: 2 start bits (not stored in Code) + toggle (not stored, always 0 here) + 5 bit address + 6 bit command
constexpr uint8_t RC5_ADDRESS_BITS = 5;
constexpr uint8_t RC5_COMMAND_BITS = 6;
constexpr uint8_t RC5_PAYLOAD_BITS = 1 + RC5_ADDRESS_BITS + RC5_COMMAND_BITS; //toggle + address + command
constexpr double RC5_CLOCK_HZ = 36000.0;
}

CodeEditor::CodeEditor(const Code &code, CodeType type, QWidget *parent) :
    QDialog(parent), codeType(type)
{
  setWindowTitle(tr("Edit IR Code"));

  stack = new QStackedWidget(this);
  stack->addWidget(createProprietaryPage());
  stack->addWidget(createRc5Page());

  switch (codeType) {
    case CodeType::PhilipsRc5:
      stack->setCurrentIndex(1);
      loadRc5(code);
      break;
    case CodeType::Proprietary:
    default:
      stack->setCurrentIndex(0);
      loadProprietary(code);
      break;
  }

  auto *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &CodeEditor::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &CodeEditor::reject);

  auto *layout = new QVBoxLayout(this);
  layout->addWidget(stack);
  layout->addWidget(buttonBox);

  resize(420, 480);
}

CodeEditor::~CodeEditor() = default;

Code CodeEditor::getCode() const
{
  switch (codeType) {
    case CodeType::PhilipsRc5:
      return getRc5Code();
    case CodeType::Proprietary:
    default:
      return getProprietaryCode();
  }
}

QWidget* CodeEditor::createProprietaryPage()
{
  auto *page = new QWidget(this);

  indexBox = new QSpinBox(page);
  indexBox->setRange(0, 65535);

  clockBox = new QSpinBox(page);
  clockBox->setRange(1, 1000000);
  clockBox->setSuffix(tr(" Hz"));

  controlTypeBox = new QComboBox(page);
  controlTypeBox->addItem(tr("Flat"));
  controlTypeBox->addItem(tr("Single Section"));
  controlTypeBox->addItem(tr("Multi Section"));
  connect(controlTypeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
      this, &CodeEditor::controlTypeChanged);

  sectionCountBox = new QSpinBox(page);
  sectionCountBox->setRange(2, 255);
  connect(sectionCountBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
      &CodeEditor::updateSectionTableRowCount);

  repeatModeBox = new QComboBox(page);
  repeatModeBox->addItem(tr("Repeat Frame"));
  repeatModeBox->addItem(tr("Tx Count"));
  connect(repeatModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
      this, &CodeEditor::repeatModeChanged);

  repeatCountBox = new QSpinBox(page);
  repeatCountBox->setRange(1, 255);

  sectionTable = new QTableWidget(0, 2, page);
  sectionTable->setHorizontalHeaderLabels( { tr("Value"), tr("Bits") });
  sectionTable->horizontalHeader()->setSectionResizeMode(0,
      QHeaderView::Stretch);
  sectionTable->verticalHeader()->setVisible(false);

  auto *formLayout = new QFormLayout();
  formLayout->addRow(tr("Index:"), indexBox);
  formLayout->addRow(tr("Clock:"), clockBox);
  formLayout->addRow(tr("Control type:"), controlTypeBox);
  formLayout->addRow(tr("Section count:"), sectionCountBox);
  formLayout->addRow(tr("Repeat mode:"), repeatModeBox);
  formLayout->addRow(tr("Tx count:"), repeatCountBox);

  auto *layout = new QVBoxLayout(page);
  layout->addLayout(formLayout);
  layout->addWidget(
      new QLabel(tr("Section data (decimal, 0x hex, or 0b binary):"), page));
  layout->addWidget(sectionTable);

  return page;
}

QWidget* CodeEditor::createRc5Page()
{
  auto *page = new QWidget(this);

  rc5AddressBox = new QSpinBox(page);
  rc5AddressBox->setRange(0, (1 << RC5_ADDRESS_BITS) - 1);

  rc5CommandBox = new QSpinBox(page);
  rc5CommandBox->setRange(0, (1 << RC5_COMMAND_BITS) - 1);

  auto *formLayout = new QFormLayout(page);
  formLayout->addRow(tr("Address:"), rc5AddressBox);
  formLayout->addRow(tr("Command:"), rc5CommandBox);

  return page;
}

void CodeEditor::loadProprietary(const Code &code)
{
  indexBox->setValue(code.getIndex());
  clockBox->setValue(static_cast<int>(code.getClock()));

  auto control = code.getControl();
  if (control == 0) {
    controlTypeBox->setCurrentIndex(0); //Flat
  } else if (control == 1) {
    controlTypeBox->setCurrentIndex(1); //Single Section
  } else {
    controlTypeBox->setCurrentIndex(2); //Multi Section
    sectionCountBox->setValue(code.getDataSectionCount());
  }

  if (code.getRepeatFrame() != 0) {
    repeatModeBox->setCurrentIndex(0);
  } else {
    repeatModeBox->setCurrentIndex(1);
    repeatCountBox->setValue(code.getDataFrameTxCount());
  }

  sectionTable->setRowCount(0);
  for (const auto &section : code.accessSections()) {
    auto row = sectionTable->rowCount();
    sectionTable->insertRow(row);

    const auto &bits = section.getData();
    uint64_t value = 0;
    for (bool bit : bits) {
      value = (value << 1) | (bit ? 1 : 0);
    }

    sectionTable->setItem(row, 0,
        new QTableWidgetItem("0x" + QString::number(value, 16).toUpper()));

    auto *bitsBox = new QSpinBox(sectionTable);
    bitsBox->setRange(1, 64);
    bitsBox->setValue(static_cast<int>(bits.size()));
    sectionTable->setCellWidget(row, 1, bitsBox);
  }

  updateSectionTableRowCount();
}

void CodeEditor::loadRc5(const Code &code)
{
  //RC5 payload is a single section: [toggle(1)][address(5)][command(6)]
  const auto &sections = code.accessSections();
  if (sections.empty()) {
    rc5AddressBox->setValue(0);
    rc5CommandBox->setValue(0);
    return;
  }

  const auto &bits = sections.front().getData();
  uint64_t value = 0;
  for (bool bit : bits) {
    value = (value << 1) | (bit ? 1 : 0);
  }

  auto command = value & ((1u << RC5_COMMAND_BITS) - 1);
  auto address = (value >> RC5_COMMAND_BITS) & ((1u << RC5_ADDRESS_BITS) - 1);

  rc5AddressBox->setValue(static_cast<int>(address));
  rc5CommandBox->setValue(static_cast<int>(command));
}

Code CodeEditor::getProprietaryCode() const
{
  Code code;

  code.setIndex(static_cast<uint16_t>(indexBox->value()));
  code.setTicks(static_cast<uint16_t>(SYSCLOCK_HZ / clockBox->value()));
  code.setControl(
      static_cast<uint8_t>(
          controlTypeBox->currentIndex() == 2 ?
              sectionCountBox->value() : controlTypeBox->currentIndex()));

  if (repeatModeBox->currentIndex() == 0) {
    code.setRepeatFrame(1);
    code.setDataFrameTxCount(1);
  } else {
    code.setRepeatFrame(0);
    code.setDataFrameTxCount(static_cast<uint8_t>(repeatCountBox->value()));
  }

  code.setDataSectionCount(static_cast<uint8_t>(sectionTable->rowCount()));

  std::vector<Section> sections;
  for (int row = 0; row < sectionTable->rowCount(); row++) {
    auto *valueItem = sectionTable->item(row, 0);
    auto *bitsBox = qobject_cast<QSpinBox*>(sectionTable->cellWidget(row, 1));
    if ((valueItem == nullptr) || (bitsBox == nullptr)) {
      continue;
    }

    bool ok = false;
    auto value = parsePrefixedValue(valueItem->text(), &ok);
    auto bitCount = static_cast<uint8_t>(bitsBox->value());

    std::vector<bool> bits;
    bits.reserve(bitCount);
    for (int i = bitCount - 1; i >= 0; i--) {
      bits.push_back((value >> i) & 1);
    }
    sections.push_back(Section(row, bits));
  }
  code.setSections(sections);

  return code;
}

Code CodeEditor::getRc5Code() const
{
  Code code;

  uint64_t payload = 0; //toggle bit is always 0 here, actual toggling happens at send time
  payload = (payload << RC5_ADDRESS_BITS)
      | static_cast<uint64_t>(rc5AddressBox->value());
  payload = (payload << RC5_COMMAND_BITS)
      | static_cast<uint64_t>(rc5CommandBox->value());

  code.createSingleSection(0, RC5_CLOCK_HZ, RC5_PAYLOAD_BITS, payload);

  return code;
}

void CodeEditor::updateSectionTableRowCount()
{
  int wanted = 1;
  if (controlTypeBox->currentIndex() == 2) {
    wanted = sectionCountBox->value();
  }

  while (sectionTable->rowCount() < wanted) {
    auto row = sectionTable->rowCount();
    sectionTable->insertRow(row);
    sectionTable->setItem(row, 0, new QTableWidgetItem("0x0"));

    auto *bitsBox = new QSpinBox(sectionTable);
    bitsBox->setRange(1, 64);
    bitsBox->setValue(16);
    sectionTable->setCellWidget(row, 1, bitsBox);
  }
  while (sectionTable->rowCount() > wanted) {
    sectionTable->removeRow(sectionTable->rowCount() - 1);
  }
}

void CodeEditor::controlTypeChanged(int index)
{
  sectionCountBox->setEnabled(index == 2);
  updateSectionTableRowCount();
}

void CodeEditor::repeatModeChanged(int index)
{
  repeatCountBox->setEnabled(index == 1);
}

uint64_t CodeEditor::parsePrefixedValue(const QString &text, bool *ok)
{
  auto trimmed = text.trimmed();

  if (trimmed.startsWith("0x", Qt::CaseInsensitive)) {
    return trimmed.mid(2).toULongLong(ok, 16);
  }
  if (trimmed.startsWith("0b", Qt::CaseInsensitive)) {
    return trimmed.mid(2).toULongLong(ok, 2);
  }
  return trimmed.toULongLong(ok, 10);
}

}
