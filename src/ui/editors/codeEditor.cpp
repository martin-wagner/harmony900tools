// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include "lib/users.h"
#include "lib/bits.h"
#include "context.h"
#include "codeEditor.h"

using namespace binary::irProto;
using namespace document::data;

namespace editors
{

CodeEditor::CodeEditor(Context &ctx, const Code &code, CodeType type,
    QWidget *parent) :
    QDialog(parent), ctx(ctx), codeType(type)
{
  if (ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    editAll = true;
  }

  setWindowTitle(tr("Edit IR Code"));

  stack = new QStackedWidget(this);
  stack->addWidget(createProprietaryPage());
  stack->addWidget(createRc5Page());

  switch (codeType) {
    case CodeType::PhilipsRC5:
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

bool CodeEditor::isSupported(CodeType type)
{
  switch (type) {
    case CodeType::Proprietary:
    case CodeType::PhilipsRC5:
      return true;
    default:
      return false;
  }
}

Code CodeEditor::createDefault(int index, CodeType type)
{
  Code code;

  switch (type) {
    case CodeType::PhilipsRC5:
      code.createSingleSection(index, RC5_CLOCK_HZ, RC5_FRAME_BITS,
          0x01 << RC5_PAYLOAD_BITS);
      code.setRepeatFrame(0);
      code.setDataFrameTxCount(3);
      break;
    case CodeType::Proprietary:
    case CodeType::None:
    case CodeType::Unknown:
    default:
      break;
  }

  return code;
}

Code CodeEditor::getCode() const
{
  switch (codeType) {
    case CodeType::PhilipsRC5:
      return getRc5Code();
    case CodeType::Proprietary:
      return getProprietaryCode();
    default:
      return Code();
  }
}

QString editors::CodeEditor::toString(document::data::CodeType type,
    const binary::irProto::Code &code)
{
  uint32_t address;
  uint32_t command;

  switch (type) {
    case CodeType::PhilipsRC5:
      if (code.getDataSectionCount() != 1) {
        return QString();
      }
      decodeRc5(code, address, command);
      return tr("Address: %1, Command: %2, Tx: %3").arg(address).arg(
          command).arg(code.getDataFrameTxCount());
    default:
      break;
  }
  return QString();
}

QWidget* CodeEditor::createProprietaryPage()
{
  auto *page = new QWidget(this);

  //base info is not editable

  indexBox = new QSpinBox(page);
  indexBox->setEnabled(false); //must match irProto, not editable

  delayBox = new QSpinBox(page);
  delayBox->setRange(0, 100000);
  delayBox->setSuffix(tr(" ms"));
  delayBox->setToolTip(
      tr("Yet another pause or delay (most likely). Not yet found "
          "where it applies. Default seems to be 500ms"));

  controlTypeBox = new QComboBox(page);
  controlTypeBox->addItem(tr("Flat"));
  controlTypeBox->addItem(tr("Single Section"));
  controlTypeBox->addItem(tr("Multi Section"));
  controlTypeBox->setEnabled(editAll); //must match irProto, not editable

  sectionCountBox = new QSpinBox(page);
  sectionCountBox->setRange(0, 25);
  sectionCountBox->setEnabled(editAll); //must match irProto, not editable

  repeatModeBox = new QComboBox(page);
  repeatModeBox->addItem(tr("Repeat Frame"));
  repeatModeBox->addItem(tr("Tx Count"));
  repeatModeBox->setEnabled(editAll); //must match irProto, not editable

  repeatCountBox = new QSpinBox(page);
  repeatCountBox->setRange(0, 25);

  sectionTable = new QTableWidget(0, 2, page);
  sectionTable->setHorizontalHeaderLabels( { tr("Value"), tr("Bits") });
  sectionTable->horizontalHeader()->setSectionResizeMode(0,
      QHeaderView::Stretch);
  sectionTable->verticalHeader()->setVisible(false);

  auto *formLayout = new QFormLayout();
  formLayout->addRow(tr("Index:"), indexBox);
  formLayout->addRow(tr("Pause/Delay:"), delayBox);
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

  rc5IndexBox = new QSpinBox(page);
  rc5IndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    rc5IndexBox->setVisible(false);
  }

  rc5AddressBox = new QSpinBox(page);
  rc5AddressBox->setRange(0, MASK(RC5_ADDRESS_BITS));

  rc5CommandBox = new QSpinBox(page);
  rc5CommandBox->setRange(0, MASK(RC5_COMMAND_BITS));

  rc5RepeatCountBox = new QSpinBox(page);
  rc5RepeatCountBox->setRange(1, 25);

  auto *formLayout = new QFormLayout(page);
  if (ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    formLayout->addRow(tr("Index:"), rc5IndexBox);
  }
  formLayout->addRow(tr("Address:"), rc5AddressBox);
  formLayout->addRow(tr("Command:"), rc5CommandBox);
  formLayout->addRow(tr("Tx count:"), rc5RepeatCountBox);

  return page;
}

void CodeEditor::loadProprietary(const Code &code)
{
  indexBox->setValue(code.getIndex());
  delayBox->setValue(code.getDelay());
  sectionCountBox->setValue(code.getDataSectionCount());

  auto control = static_cast<Code::Ctrl>(code.getControl());
  switch (control) {
    case Code::Ctrl::FLAT:
      controlTypeBox->setCurrentIndex(0); //Flat
      break;
    case Code::Ctrl::SECTIONS_1:
      controlTypeBox->setCurrentIndex(1); //Single Section
      break;
    default:
      controlTypeBox->setCurrentIndex(2); //Multi Section
      break;
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
    auto value = lib::bitsTou64Msb(bits);

    sectionTable->setItem(row, 0,
        new QTableWidgetItem("0x" + QString::number(value, 16).toUpper()));

    auto *bitsBox = new QSpinBox(sectionTable);
    bitsBox->setRange(1, 64);
    bitsBox->setValue(static_cast<int>(bits.size()));
    bitsBox->setEnabled(editAll); //must match irProto, not editable
    sectionTable->setCellWidget(row, 1, bitsBox);
  }
}

void CodeEditor::loadRc5(const Code &code)
{
  uint32_t address;
  uint32_t command;

  decodeRc5(code, address, command);

  rc5IndexBox->setValue(code.getIndex());
  rc5AddressBox->setValue(address);
  rc5CommandBox->setValue(command);
  rc5RepeatCountBox->setValue(code.getDataFrameTxCount());
}

void CodeEditor::decodeRc5(const binary::irProto::Code &code, uint32_t &address,
    uint32_t &command)
{
  //RC5 payload is a single section: [start(2)][toggle(1)][address(5)][command(6)]
  const auto &sections = code.accessSections();
  if (sections.empty()) {
    address = 0;
    command = 0;
    return;
  }

  const auto &bits = sections.front().getData();
  auto value = lib::bitsTou64Msb(bits);

  address = (value >> RC5_COMMAND_BITS) & MASK(RC5_ADDRESS_BITS);
  command = value & MASK(RC5_COMMAND_BITS);
}

Code CodeEditor::getProprietaryCode() const
{
  Code code;

  code.setIndex(static_cast<uint16_t>(indexBox->value()));
  code.setDelay(static_cast<uint16_t>(delayBox->value()));
  if (controlTypeBox->currentIndex() == 2) {
    code.setControl(sectionCountBox->value()); // >= 2
  } else {
    code.setControl(controlTypeBox->currentIndex());
  }

  if (repeatModeBox->currentIndex() == 0) {
    code.setRepeatFrame(1);
    code.setDataFrameTxCount(1);
  } else {
    code.setRepeatFrame(0);
    code.setDataFrameTxCount(static_cast<uint8_t>(repeatCountBox->value()));
  }

  std::vector<Section> sections;
  for (int row = 0; row < sectionTable->rowCount(); row++) {
    auto *valueItem = sectionTable->item(row, 0);
    auto *bitsBox = qobject_cast<QSpinBox*>(sectionTable->cellWidget(row, 1));
    if ((valueItem == nullptr) || (bitsBox == nullptr)) {
      continue;
    }

    auto value = parsePrefixedValue(valueItem->text());
    auto bitCount = static_cast<uint8_t>(bitsBox->value());
    auto bits = lib::u64ToBitsMsb(bitCount, value);
    sections.push_back(Section(row, bits));
  }
  code.setSections(sections);

  return code;
}

Code CodeEditor::getRc5Code() const
{
  Code code;

  uint64_t payload = 0x01 << RC5_PAYLOAD_BITS; // 1 software start bits (1), 1 toggle bit (not placed here)
  payload |= static_cast<uint64_t>(rc5AddressBox->value()) << RC5_COMMAND_BITS;
  payload |= static_cast<uint64_t>(rc5CommandBox->value());

  code.createSingleSection(rc5IndexBox->value(), RC5_CLOCK_HZ, RC5_FRAME_BITS,
      payload);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(rc5RepeatCountBox->value());
  return code;
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
