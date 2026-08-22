// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLineEdit>

#include "lib/users.h"
#include "bin/codec/encode.h"
#include "context.h"
#include "codeEditor.h"

using namespace binary::irProto;
using namespace document::data;
using namespace std;

namespace editors
{

BaseSpinBox::BaseSpinBox(QWidget *parent) :
    QSpinBox(parent)
{
}

void BaseSpinBox::setDisplayBase(Base newBase)
{
  displayBase = newBase;
  //re-render the currently shown text in the new base
  lineEdit()->setText(textFromValue(value()));
}

QValidator::State BaseSpinBox::validate(QString &input, int &pos) const
{
  Q_UNUSED(pos);

  auto trimmed = input.trimmed();
  if (trimmed.isEmpty()) {
    return QValidator::Intermediate;
  }

  bool ok = false;
  if (trimmed.startsWith("0x", Qt::CaseInsensitive)) {
    auto digits = trimmed.mid(2);
    if (digits.isEmpty()) {
      return QValidator::Intermediate; //user is still typing the prefix
    }
    digits.toULongLong(&ok, 16);
  } else if (trimmed.startsWith("0b", Qt::CaseInsensitive)) {
    auto digits = trimmed.mid(2);
    if (digits.isEmpty()) {
      return QValidator::Intermediate; //user is still typing the prefix
    }
    digits.toULongLong(&ok, 2);
  } else if ((trimmed == "0") || (QString("0b").startsWith(trimmed, Qt::CaseInsensitive))
      || (QString("0x").startsWith(trimmed, Qt::CaseInsensitive))) {
    return QValidator::Intermediate; //user could still be typing "0x"/"0b"
  } else {
    trimmed.toULongLong(&ok, 10);
  }

  if (!ok) {
    return QValidator::Invalid;
  }
  return QValidator::Acceptable;
}

int BaseSpinBox::valueFromText(const QString &text) const
{
  auto trimmed = text.trimmed();

  bool ok = false;
  uint64_t result = 0;
  if (trimmed.startsWith("0x", Qt::CaseInsensitive)) {
    result = trimmed.mid(2).toULongLong(&ok, 16);
  } else if (trimmed.startsWith("0b", Qt::CaseInsensitive)) {
    result = trimmed.mid(2).toULongLong(&ok, 2);
  } else {
    result = trimmed.toULongLong(&ok, 10);
  }

  if (!ok) {
    return 0;
  }
  return static_cast<int>(result);
}

QString BaseSpinBox::textFromValue(int value) const
{
  switch (displayBase) {
    case Base::Hex:
      return "0x" + QString::number(static_cast<uint32_t>(value), 16).toUpper();
    case Base::Binary:
      return "0b" + QString::number(static_cast<uint32_t>(value), 2);
    default:
      return QString::number(value);
  }
}

editors::CodeEditor::CodeEditor(Context &ctx, CodeType codeType,
    const QString &codeString, uint32_t address, uint32_t command,
    const std::vector<bool> &rawData, const Code &code, QWidget *parent) :
    QDialog(parent), ctx(ctx), codeType(codeType), codeString(codeString), address(
        address), command(command), rawData(rawData), code(code)
{
  if (ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    editAll = true;
  }

  setWindowTitle(tr("Edit IR Code"));

  stack = new QStackedWidget(this);
  stack->addWidget(createProprietaryPage());
  stack->addWidget(createNecPage());
  stack->addWidget(createKasPage());
  stack->addWidget(createS20Page());
  stack->addWidget(createACPage());

  switch (codeType) {
    case CodeType::Proprietary:
      stack->setCurrentIndex(0);
      loadProprietary();
      break;
    case CodeType::NEC:
      stack->setCurrentIndex(1);
      loadNec();
      break;
    case CodeType::KASEIKYO:
      stack->setCurrentIndex(2);
      loadKas();
      break;
    case CodeType::SIRCS20:
      stack->setCurrentIndex(3);
      loadS20();
      break;
    default:
      stack->setCurrentIndex(4);
      loadAC();
      break;
  }

  auto *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &CodeEditor::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &CodeEditor::reject);

  displayBaseBox = new QComboBox(this);
  displayBaseBox->addItem(tr("Decimal"));
  displayBaseBox->addItem(tr("Hex"));
  displayBaseBox->addItem(tr("Binary"));
  connect(displayBaseBox, &QComboBox::currentIndexChanged, this,
      [this](int index) {
        setDisplayBase(static_cast<BaseSpinBox::Base>(index));
      });

  auto *displayBaseLayout = new QHBoxLayout();
  displayBaseLayout->addWidget(new QLabel(tr("Display as:"), this));
  displayBaseLayout->addWidget(displayBaseBox);
  displayBaseLayout->addStretch();

  auto *layout = new QVBoxLayout(this);
  layout->addLayout(displayBaseLayout);
  layout->addWidget(stack);
  layout->addWidget(buttonBox);

  resize(420, 480);
}

CodeEditor::~CodeEditor() = default;

void CodeEditor::setDisplayBase(BaseSpinBox::Base newBase)
{
  for (auto *spinBox : baseSpinBoxes) {
    spinBox->setDisplayBase(newBase);
  }
}

bool CodeEditor::isSupported(CodeType type)
{
  switch (type) {
    case CodeType::Proprietary:
    case CodeType::NEC:
    case CodeType::KASEIKYO:
    case CodeType::SIRCS12:
    case CodeType::SIRCS15:
    case CodeType::SIRCS20:
    case CodeType::Samsung32:
    case CodeType::PhilipsRC5:
    case CodeType::PhilipsRC6:
    case CodeType::PhilipsRC6A:
      return true;
    default:
      return false;
  }
}

void CodeEditor::accept()
{
  switch (codeType) {
    case CodeType::Proprietary:
      updateProprietaryData();
      break;
    case CodeType::NEC:
      updateNecData();
      break;
    case CodeType::KASEIKYO:
      updateKasData();
      break;
    case CodeType::SIRCS20:
      updateS20Data();
      break;
    default:
      updateACData();
      break;
  }
  QDialog::accept();
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

QWidget* CodeEditor::createNecPage()
{
  auto updateLimits = [this](const QString &text) {
    uint32_t addressLimit;
    uint32_t commandLimit;

    binary::codec::getCodeSize(codeType, text, addressLimit, commandLimit);
    addressLimit = MASK(addressLimit);
    commandLimit = MASK(commandLimit);

    necAddressBox->setRange(0, addressLimit);
    necCommandBox->setRange(0, commandLimit);
  };

  auto *page = new QWidget(this);

  necIndexBox = new QSpinBox(page);
  necIndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    necIndexBox->setVisible(false);
  }

  necDelayBox = new QSpinBox(page);
  necDelayBox->setRange(0, 100000);
  necDelayBox->setSuffix(tr(" ms"));
  necDelayBox->setToolTip(
      tr("Yet another pause or delay (most likely). Not yet found "
          "where it applies. Default seems to be 500ms"));

  necSubTypeBox = new QComboBox(page);
  necSubTypeBox->addItems(binary::codec::getSubTypes(codeType));
  necSubTypeBox->setCurrentText(codeString);
  connect(necSubTypeBox, &QComboBox::currentTextChanged, this, updateLimits);

  necAddressBox = new BaseSpinBox(page);
  necCommandBox = new BaseSpinBox(page);
  baseSpinBoxes.push_back(necAddressBox);
  baseSpinBoxes.push_back(necCommandBox);
  updateLimits(codeString);

  auto *formLayout = new QFormLayout(page);
  if (ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    formLayout->addRow(tr("Index:"), necIndexBox);
  }
  formLayout->addRow(tr("Pause/Delay:"), necDelayBox);
  formLayout->addRow(tr("Subtype:"), necSubTypeBox);
  formLayout->addRow(tr("Address:"), necAddressBox);
  formLayout->addRow(tr("Command:"), necCommandBox);

  return page;
}

QWidget* CodeEditor::createKasPage()
{
  auto *page = new QWidget(this);

  kasIndexBox = new QSpinBox(page);
  kasIndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    kasIndexBox->setVisible(false);
  }

  kasDelayBox = new QSpinBox(page);
  kasDelayBox->setRange(0, 100000);
  kasDelayBox->setSuffix(tr(" ms"));
  kasDelayBox->setToolTip(
      tr("Yet another pause or delay (most likely). Not yet found "
          "where it applies. Default seems to be 500ms"));

  kasSubTypeBox = new QComboBox(page);
  kasSubTypeBox->addItems(binary::codec::getSubTypes(codeType));
  kasSubTypeBox->setCurrentText(codeString);
  //keine Auswirkung auf Wertebereich

  kasMnfBox = new BaseSpinBox(page);
  kasMnfBox->setRange(0, 65535); //16 bit

  kasDeviceBox = new BaseSpinBox(page);
  kasDeviceBox->setRange(0, 255);

  kasSubdeviceBox = new BaseSpinBox(page);
  kasSubdeviceBox->setRange(0, 255);

  kasCommandBox = new BaseSpinBox(page);
  kasCommandBox->setRange(0, 255);

  baseSpinBoxes.push_back(kasMnfBox);
  baseSpinBoxes.push_back(kasDeviceBox);
  baseSpinBoxes.push_back(kasSubdeviceBox);
  baseSpinBoxes.push_back(kasCommandBox);

  kasRepeatCountBox = new QSpinBox(page);
  kasRepeatCountBox->setRange(1, 25);

  auto *formLayout = new QFormLayout(page);
  if (ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    formLayout->addRow(tr("Index:"), kasIndexBox);
  }
  formLayout->addRow(tr("Subtype:"), kasSubTypeBox);
  formLayout->addRow(tr("Pause/Delay:"), kasDelayBox);
  formLayout->addRow(tr("Manufacturer:"), kasMnfBox);
  formLayout->addRow(tr("Device:"), kasDeviceBox);
  formLayout->addRow(tr("Subdevice:"), kasSubdeviceBox);
  formLayout->addRow(tr("Command:"), kasCommandBox);
  formLayout->addRow(tr("Repeat count:"), kasRepeatCountBox);

  return page;
}

QWidget* CodeEditor::createS20Page()
{
  uint32_t addressLimit;
  uint32_t commandLimit;

  binary::codec::getCodeSize(codeType, codeString, addressLimit, commandLimit);
  addressLimit = MASK(addressLimit);
  commandLimit = MASK(commandLimit);
  uint32_t extraLimit = 255;

  auto *page = new QWidget(this);

  s20IndexBox = new QSpinBox(page);
  s20IndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    s20IndexBox->setVisible(false);
  }

  s20DelayBox = new QSpinBox(page);
  s20DelayBox->setRange(0, 100000);
  s20DelayBox->setSuffix(tr(" ms"));
  s20DelayBox->setToolTip(
      tr("Yet another pause or delay (most likely). Not yet found "
          "where it applies. Default seems to be 500ms"));

  s20SubTypeBox = new QComboBox(page);
  s20SubTypeBox->addItems(binary::codec::getSubTypes(codeType));

  s20AddressBox = new BaseSpinBox(page);
  s20AddressBox->setRange(0, addressLimit);
  s20CommandBox = new BaseSpinBox(page);
  s20CommandBox->setRange(0, commandLimit);
  s20ExtraBox = new BaseSpinBox(page);
  s20ExtraBox->setRange(0, extraLimit);

  baseSpinBoxes.push_back(s20AddressBox);
  baseSpinBoxes.push_back(s20CommandBox);
  baseSpinBoxes.push_back(s20ExtraBox);

  s20RepeatCountBox = new QSpinBox(page);
  s20RepeatCountBox->setRange(1, 25);

  auto *formLayout = new QFormLayout(page);
  if (ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    formLayout->addRow(tr("Index:"), s20IndexBox);
  }
  formLayout->addRow(tr("Pause/Delay:"), s20DelayBox);
  formLayout->addRow(tr("Subtype:"), s20SubTypeBox);
  formLayout->addRow(tr("Address:"), s20AddressBox);
  formLayout->addRow(tr("Command:"), s20CommandBox);
  formLayout->addRow(tr("Extra:"), s20ExtraBox);
  formLayout->addRow(tr("Tx count:"), s20RepeatCountBox);

  return page;
}

QWidget* CodeEditor::createACPage()
{
  auto updateLimits = [this](const QString &text) {
    uint32_t addressLimit;
    uint32_t commandLimit;

    binary::codec::getCodeSize(codeType, text, addressLimit, commandLimit);
    addressLimit = MASK(addressLimit);
    commandLimit = MASK(commandLimit);

    acAddressBox->setRange(0, addressLimit);
    acCommandBox->setRange(0, commandLimit);
  };

  auto *page = new QWidget(this);

  acIndexBox = new QSpinBox(page);
  acIndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    acIndexBox->setVisible(false);
  }

  acDelayBox = new QSpinBox(page);
  acDelayBox->setRange(0, 100000);
  acDelayBox->setSuffix(tr(" ms"));
  acDelayBox->setToolTip(
      tr("Yet another pause or delay (most likely). Not yet found "
          "where it applies. Default seems to be 500ms"));

  acSubTypeBox = new QComboBox(page);
  acSubTypeBox->addItems(binary::codec::getSubTypes(codeType));
  connect(acSubTypeBox, &QComboBox::currentTextChanged, this, updateLimits);

  acAddressBox = new BaseSpinBox(page);
  acCommandBox = new BaseSpinBox(page);
  baseSpinBoxes.push_back(acAddressBox);
  baseSpinBoxes.push_back(acCommandBox);
  updateLimits(codeString);

  acRepeatCountBox = new QSpinBox(page);
  acRepeatCountBox->setRange(1, 25);

  auto *formLayout = new QFormLayout(page);
  if (ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    formLayout->addRow(tr("Index:"), acIndexBox);
  }
  formLayout->addRow(tr("Pause/Delay:"), acDelayBox);
  formLayout->addRow(tr("Subtype:"), acSubTypeBox);
  formLayout->addRow(tr("Address:"), acAddressBox);
  formLayout->addRow(tr("Command:"), acCommandBox);
  formLayout->addRow(tr("Tx count:"), acRepeatCountBox);

  return page;
}

void CodeEditor::loadProprietary()
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
    auto value = binary::irProto::Code::bitsToU64(bits);

    sectionTable->setItem(row, 0,
        new QTableWidgetItem("0x" + QString::number(value, 16).toUpper()));

    auto *bitsBox = new QSpinBox(sectionTable);
    bitsBox->setRange(1, 64);
    bitsBox->setValue(static_cast<int>(bits.size()));
    bitsBox->setEnabled(editAll); //must match irProto, not editable
    sectionTable->setCellWidget(row, 1, bitsBox);
  }
}

void CodeEditor::loadNec()
{
  necIndexBox->setValue(code.getIndex());
  necDelayBox->setValue(code.getDelay());
  necSubTypeBox->setCurrentText(codeString);
  necAddressBox->setValue(address);
  necCommandBox->setValue(command);
}

void CodeEditor::loadKas()
{
  uint64_t data = binary::irProto::Code::bitsToU64(rawData);

  kasIndexBox->setValue(code.getIndex());
  kasDelayBox->setValue(code.getDelay());
  kasMnfBox->setValue(address);
  kasDeviceBox->setValue((data >> 24) & 0xff);
  kasSubdeviceBox->setValue((data >> 16) & 0xff);
  kasCommandBox->setValue((data >> 8) & 0xff);
  kasRepeatCountBox->setValue(code.getDataFrameTxCount());
}

void CodeEditor::loadS20()
{
  s20IndexBox->setValue(code.getIndex());
  s20DelayBox->setValue(code.getDelay());
  s20SubTypeBox->setCurrentText(codeString);
  s20AddressBox->setValue(address);
  s20CommandBox->setValue(command & 0x7f);
  s20ExtraBox->setValue((command >> 7) & 0xff);
  s20RepeatCountBox->setValue(code.getDataFrameTxCount());
}

void CodeEditor::loadAC()
{
  acIndexBox->setValue(code.getIndex());
  acDelayBox->setValue(code.getDelay());
  acSubTypeBox->setCurrentText(codeString);
  acAddressBox->setValue(address);
  acCommandBox->setValue(command);
  acRepeatCountBox->setValue(code.getDataFrameTxCount());
}

void CodeEditor::updateProprietaryData()
{
  code = Code(); //wipe
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
    auto bits = binary::irProto::Code::u64tobits(bitCount, value);
    sections.push_back(Section(row, bits));
  }
  code.setSections(sections);
}

void CodeEditor::updateNecData()
{
  codeString = necSubTypeBox->currentText();
  address = necAddressBox->value();
  command = necCommandBox->value();
  code = binary::codec::encode(necIndexBox->value(), codeType,
      codeString.toStdString(), address, command, rawData);
  code.setDelay(static_cast<uint16_t>(necDelayBox->value()));
}

void CodeEditor::updateKasData()
{
  uint64_t data = 0;

  codeString = kasSubTypeBox->currentText();
  address = kasMnfBox->value();
  command = 0;
  data = data | (kasDeviceBox->value() << 24);
  data = data | (kasSubdeviceBox->value() << 16);
  data = data | (kasCommandBox->value() << 8);
  //parity byte not required for encoding
  rawData = binary::irProto::Code::u64tobits(48, data);
  code = binary::codec::encode(kasIndexBox->value(), codeType,
      codeString.toStdString(), address, command, rawData);
  code.setDataFrameTxCount(kasRepeatCountBox->value());
  code.setDelay(static_cast<uint16_t>(kasDelayBox->value()));
}

void CodeEditor::updateS20Data()
{
  codeString = s20SubTypeBox->currentText();
  address = s20AddressBox->value();
  command = (s20CommandBox->value() & 0x7f)
      | ((s20ExtraBox->value() & 0xff) << 7);
  code = binary::codec::encode(s20IndexBox->value(), codeType,
      codeString.toStdString(), address, command, rawData);
  code.setDataFrameTxCount(s20RepeatCountBox->value());
  code.setDelay(static_cast<uint16_t>(s20DelayBox->value()));
}

void CodeEditor::updateACData()
{
  codeString = acSubTypeBox->currentText();
  address = acAddressBox->value();
  command = acCommandBox->value();
  code = binary::codec::encode(acIndexBox->value(), codeType,
      codeString.toStdString(), address, command, rawData);
  code.setDataFrameTxCount(acRepeatCountBox->value());
  code.setDelay(static_cast<uint16_t>(acDelayBox->value()));
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
