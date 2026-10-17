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
#include <QGroupBox>
#include <QMessageBox>

#include "lib/users.h"
#include "bin/codec/encode.h"
#include "bin/irProto/file.h"
#include "document/config.h"
#include "context.h"
#include "codeEditor.h"

using namespace binary::irProto;
using namespace document::data;
using namespace std;

namespace editors
{

/** symbols (MSB first) -> bit list */
vector<uint8_t> symbolsToBits(const vector<uint8_t> &symbols,
    uint8_t bitsPerSymbol)
{
  vector<uint8_t> bits;

  for (auto symbol : symbols) {
    for (int bit = bitsPerSymbol - 1; bit >= 0; --bit) {
      bits.push_back((symbol >> bit) & 1);
    }
  }
  return bits;
}

/**
 * symbol list -> text of the bit stream (not padded to bytes) in the given base.
 * Decimal can't show more than 64 bits, those are shown as hex.
 */
QString formatValue(const vector<uint8_t> &symbols, uint8_t bitsPerSymbol,
    BaseSpinBox::Base base)
{
  auto bits = symbolsToBits(symbols, bitsPerSymbol);
  if (bits.empty()) {
    return QString();
  }

  if ((base == BaseSpinBox::Base::Decimal) && (bits.size() > 64)) {
    base = BaseSpinBox::Base::Hex;
  }

  if (base == BaseSpinBox::Base::Binary) {
    QString text = "0b";
    for (auto bit : bits) {
      text += QString::number(bit);
    }
    return text;
  }

  if (base == BaseSpinBox::Base::Decimal) {
    uint64_t value = 0;
    for (auto bit : bits) {
      value = (value << 1) | bit;
    }
    return QString::number(value);
  }

  int digits = (bits.size() + 3) / 4;
  int pad = digits * 4 - bits.size();
  QString text = "0x";

  for (int digit = 0; digit < digits; digit++) {
    int nibble = 0;
    for (int bit = 0; bit < 4; bit++) {
      int index = digit * 4 + bit - pad;
      nibble = nibble << 1;
      if (index >= 0) {
        nibble = nibble | bits[index];
      }
    }
    text += QString::number(nibble, 16).toUpper();
  }
  return text;
}

/**
 * "1234" / "0x1234" / "0b1010" -> symbol list.
 *
 * @return false when the text is invalid or the value does not fit into the symbols
 */
bool parseValue(const QString &text, uint16_t symbolCount,
    uint8_t bitsPerSymbol, vector<uint8_t> &symbols)
{
  vector<uint8_t> bits;
  size_t totalBits = symbolCount * bitsPerSymbol;
  auto trimmed = text.trimmed();

  symbols.clear();
  if (totalBits == 0) {
    return true;
  }

  if (trimmed.startsWith("0x", Qt::CaseInsensitive)) {
    auto digits = trimmed.mid(2);
    if (digits.isEmpty()) {
      return false;
    }
    for (const auto &c : digits) {
      bool ok = false;
      int nibble = QString(c).toInt(&ok, 16);
      if (!ok) {
        return false;
      }
      for (int bit = 3; bit >= 0; --bit) {
        bits.push_back((nibble >> bit) & 1);
      }
    }
  } else if (trimmed.startsWith("0b", Qt::CaseInsensitive)) {
    auto digits = trimmed.mid(2);
    if (digits.isEmpty()) {
      return false;
    }
    for (const auto &c : digits) {
      if (c == '0') {
        bits.push_back(0);
      } else if (c == '1') {
        bits.push_back(1);
      } else {
        return false;
      }
    }
  } else {
    bool ok = false;
    uint64_t value = trimmed.toULongLong(&ok, 10);
    if (!ok) {
      return false;
    }
    for (int bit = 63; bit >= 0; --bit) {
      bits.push_back((value >> bit) & 1);
    }
  }

  //fit to symbol bits. leading zeros are free, anything else doesn't fit.
  if (bits.size() > totalBits) {
    size_t extra = bits.size() - totalBits;
    for (size_t i = 0; i < extra; i++) {
      if (bits[i] != 0) {
        return false;
      }
    }
    bits.erase(bits.begin(), bits.begin() + extra);
  } else if (bits.size() < totalBits) {
    bits.insert(bits.begin(), totalBits - bits.size(), 0);
  }

  for (uint16_t symbol = 0; symbol < symbolCount; symbol++) {
    uint8_t value = 0;
    for (uint8_t bit = 0; bit < bitsPerSymbol; bit++) {
      value = static_cast<uint8_t>((value << 1)
          | bits[symbol * bitsPerSymbol + bit]);
    }
    symbols.push_back(value);
  }
  return true;
}

/** table cell that is shown, but can't be modified */
QTableWidgetItem* readOnlyItem(const QString &text)
{
  auto *item = new QTableWidgetItem(text);
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  return item;
}

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
  } else if ((trimmed == "0")
      || (QString("0b").startsWith(trimmed, Qt::CaseInsensitive))
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
    const std::vector<uint8_t> &rawData, const Code &code, QWidget *parent) :
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
    case CodeType::NEC40kHz:
      stack->setCurrentIndex(1);
      loadNec();
      break;
    case CodeType::KASEIKYO:
    case CodeType::KASEIKYO36kHz:
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

  if (codeType == CodeType::Proprietary) {
    //raw frames are easier to read as hex
    displayBaseBox->setCurrentIndex(static_cast<int>(BaseSpinBox::Base::Hex));
  }

  auto *displayBaseLayout = new QHBoxLayout();
  displayBaseLayout->addWidget(new QLabel(tr("Display as:"), this));
  displayBaseLayout->addWidget(displayBaseBox);
  displayBaseLayout->addStretch();

  auto *layout = new QVBoxLayout(this);
  layout->addLayout(displayBaseLayout);
  layout->addWidget(stack);
  layout->addWidget(buttonBox);

  resize(480, 640);
}

CodeEditor::~CodeEditor() = default;

bool CodeEditor::isSupported(CodeType type)
{
  switch (type) {
    case CodeType::Proprietary:
    case CodeType::NEC:
    case CodeType::NEC40kHz:
    case CodeType::KASEIKYO:
    case CodeType::KASEIKYO36kHz:
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
      if (!updateProprietaryData()) {
        return;
      }
      break;
    case CodeType::NEC:
    case CodeType::NEC40kHz:
      updateNecData();
      break;
    case CodeType::KASEIKYO:
    case CodeType::KASEIKYO36kHz:
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

void CodeEditor::setDisplayBase(BaseSpinBox::Base newBase)
{
  displayBase = newBase;

  for (auto *spinBox : baseSpinBoxes) {
    spinBox->setDisplayBase(newBase);
  }

  //group values follow the display base too
  reformatGroupTable(startTable);
  reformatGroupTable(repeatTable);
  reformatGroupTable(finishTable);
}

QWidget* CodeEditor::createProprietaryPage()
{
  auto *page = new QWidget(this);

  //base info is not editable

  indexBox = new QSpinBox(page);
  indexBox->setRange(0, Code::INVALID_INDEX_MARKER - 1);
  indexBox->setEnabled(false); //must match irProto, not editable

  delayBox = new QSpinBox(page);
  delayBox->setRange(0, 65535);
  delayBox->setSingleStep(100);
  delayBox->setSuffix(tr(" ms"));
  delayBox->setToolTip(
      tr("Minimum pause before the command is sent. Default is 500ms"));

  repeatCountBox = new QSpinBox(page);
  repeatCountBox->setRange(0, 255);
  repeatCountBox->setToolTip(
      tr("How often the repeat group is sent after the start group"));

  startTable = createGroupTable(page);
  repeatTable = createGroupTable(page);
  finishTable = createGroupTable(page);

  auto *formLayout = new QFormLayout();
  formLayout->addRow(tr("Index:"), indexBox);
  formLayout->addRow(tr("Pause/Delay:"), delayBox);
  formLayout->addRow(tr("Repeats:"), repeatCountBox);

  auto *startBox = new QGroupBox(tr("Start (sent once at the beginning)"),
      page);
  auto *startLayout = new QVBoxLayout(startBox);
  startLayout->addWidget(startTable);

  auto *repeatBox = new QGroupBox(tr("Repeat (sent for every repeat)"), page);
  auto *repeatLayout = new QVBoxLayout(repeatBox);
  repeatLayout->addWidget(repeatTable);

  auto *finishBox = new QGroupBox(tr("Finish (sent once at the end)"), page);
  auto *finishLayout = new QVBoxLayout(finishBox);
  finishLayout->addWidget(finishTable);

  auto *layout = new QVBoxLayout(page);
  layout->addLayout(formLayout);
  layout->addWidget(
      new QLabel(tr("Section data (decimal, 0x hex, or 0b binary):"), page));
  layout->addWidget(startBox);
  layout->addWidget(repeatBox);
  layout->addWidget(finishBox);

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
  necIndexBox->setRange(0, Code::INVALID_INDEX_MARKER - 1);
  necIndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    necIndexBox->setVisible(false);
  }

  necDelayBox = new QSpinBox(page);
  necDelayBox->setRange(0, 65535);
  necDelayBox->setSingleStep(100);
  necDelayBox->setSuffix(tr(" ms"));
  necDelayBox->setToolTip(
      tr("Minimum pause before the command is sent. Default is 500ms"));

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
  kasIndexBox->setRange(0, Code::INVALID_INDEX_MARKER - 1);
  kasIndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    kasIndexBox->setVisible(false);
  }

  kasDelayBox = new QSpinBox(page);
  kasDelayBox->setRange(0, 65535);
  kasDelayBox->setSingleStep(100);
  kasDelayBox->setSuffix(tr(" ms"));
  kasDelayBox->setToolTip(
      tr("Minimum pause before the command is sent. Default is 500ms"));

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
  formLayout->addRow(tr("Repeats:"), kasRepeatCountBox);

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
  s20IndexBox->setRange(0, Code::INVALID_INDEX_MARKER - 1);
  s20IndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    s20IndexBox->setVisible(false);
  }

  s20DelayBox = new QSpinBox(page);
  s20DelayBox->setRange(0, 65535);
  s20DelayBox->setSingleStep(100);
  s20DelayBox->setSuffix(tr(" ms"));
  s20DelayBox->setToolTip(
      tr("Minimum pause before the command is sent. Default is 500ms"));

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
  formLayout->addRow(tr("Repeats:"), s20RepeatCountBox);

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
  acIndexBox->setRange(0, Code::INVALID_INDEX_MARKER - 1);
  acIndexBox->setEnabled(false); //must match user data entry, not editable
  if (!ctx.userLevel().validate(lib::UserLevel::Level::Developer)) {
    acIndexBox->setVisible(false);
  }

  acDelayBox = new QSpinBox(page);
  acDelayBox->setRange(0, 65535);
  acDelayBox->setSingleStep(100);
  acDelayBox->setSuffix(tr(" ms"));
  acDelayBox->setToolTip(
      tr("Minimum pause before the command is sent. Default is 500ms"));

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
  formLayout->addRow(tr("Repeats:"), acRepeatCountBox);

  return page;
}

QTableWidget* CodeEditor::createGroupTable(QWidget *parent)
{
  auto *table = new QTableWidget(0, 4, parent);

  table->setHorizontalHeaderLabels(
      { tr("Section"), tr("Symbols"), tr("Bits/symbol"), tr("Value") });
  table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  table->verticalHeader()->setVisible(false);
  table->setMinimumHeight(90);
  //technical detail, only needed for development
  table->setColumnHidden(2, !editAll);

  return table;
}

void CodeEditor::loadProprietary()
{
  indexBox->setValue(code.getIndex());
  delayBox->setValue(code.getDelay());
  repeatCountBox->setValue(code.getRepeats());

  loadGroupTable(startTable, code.accessStartGroups());
  loadGroupTable(repeatTable, code.accessRepeatGroups());
  loadGroupTable(finishTable, code.accessFinishGroups());
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
  uint64_t data = binary::irProto::Code::listToU64bits(rawData);

  kasIndexBox->setValue(code.getIndex());
  kasDelayBox->setValue(code.getDelay());
  kasMnfBox->setValue(address);
  kasDeviceBox->setValue((data >> 24) & 0xff);
  kasSubdeviceBox->setValue((data >> 16) & 0xff);
  kasCommandBox->setValue((data >> 8) & 0xff);
  kasRepeatCountBox->setValue(code.getRepeats());
}

void CodeEditor::loadS20()
{
  s20IndexBox->setValue(code.getIndex());
  s20DelayBox->setValue(code.getDelay());
  s20SubTypeBox->setCurrentText(codeString);
  s20AddressBox->setValue(address);
  s20CommandBox->setValue(command & 0x7f);
  s20ExtraBox->setValue((command >> 7) & 0xff);
  s20RepeatCountBox->setValue(code.getRepeats());
}

void CodeEditor::loadAC()
{
  acIndexBox->setValue(code.getIndex());
  acDelayBox->setValue(code.getDelay());
  acSubTypeBox->setCurrentText(codeString);
  acAddressBox->setValue(address);
  acCommandBox->setValue(command);
  acRepeatCountBox->setValue(code.getRepeats());
}

void CodeEditor::loadGroupTable(QTableWidget *table,
    const std::vector<Group> &groups)
{
  table->setRowCount(0);

  for (const auto &group : groups) {
    auto row = table->rowCount();
    table->insertRow(row);

    //layout is defined by the protocol, show only
    table->setItem(row, 0, readOnlyItem(QString::number(group.getSection())));
    table->setItem(row, 1,
        readOnlyItem(QString::number(group.getData().size())));
    table->setItem(row, 2, readOnlyItem(QString::number(group.getBits())));

    if (group.getData().empty() || (group.getBits() == 0)) {
      //e.g. repeat frame without payload
      table->setItem(row, 3, readOnlyItem(tr("(no data)")));
    } else if ((group.getBits() == 1) && (group.getData().size() <= 64)) {
      table->setItem(row, 3,
          new QTableWidgetItem(
              formatValue(group.getData(), group.getBits(), displayBase)));
    } else {
      //too complex for this editor, show only. data is kept as it is.
      auto *item = readOnlyItem(
          formatValue(group.getData(), group.getBits(), displayBase));
      item->setToolTip(tr("Only up to 64 boolean symbols can be edited here"));
      table->setItem(row, 3, item);
    }
  }
}

void CodeEditor::reformatGroupTable(QTableWidget *table)
{
  if (table == nullptr) {
    return;
  }

  for (int row = 0; row < table->rowCount(); row++) {
    auto *symbolsItem = table->item(row, 1);
    auto *bitsItem = table->item(row, 2);
    auto *valueItem = table->item(row, 3);
    if ((symbolsItem == nullptr) || (bitsItem == nullptr)
        || (valueItem == nullptr)) {
      continue;
    }

    auto symbolCount = static_cast<uint16_t>(symbolsItem->text().toUInt());
    auto bits = static_cast<uint8_t>(bitsItem->text().toUInt());
    if ((symbolCount == 0) || (bits == 0)) {
      continue;
    }

    //whatever base the text is in, it carries its own prefix. an invalid text stays as it is.
    std::vector<uint8_t> symbols;
    if (parseValue(valueItem->text(), symbolCount, bits, symbols)) {
      valueItem->setText(formatValue(symbols, bits, displayBase));
    }
  }
}

bool CodeEditor::updateProprietaryData()
{
  auto newStart = code.accessStartGroups();
  auto newRepeat = code.accessRepeatGroups();
  auto newFinish = code.accessFinishGroups();

  if (!readGroupTable(startTable, newStart, tr("Start"))) {
    return false;
  }
  if (!readGroupTable(repeatTable, newRepeat, tr("Repeat"))) {
    return false;
  }
  if (!readGroupTable(finishTable, newFinish, tr("Finish"))) {
    return false;
  }

  //index and group layout belong to the protocol and stay as they are
  code.setDelay(static_cast<uint16_t>(delayBox->value()));
  code.setRepeats(static_cast<uint8_t>(repeatCountBox->value()));
  code.accessStartGroups() = newStart;
  code.accessRepeatGroups() = newRepeat;
  code.accessFinishGroups() = newFinish;
  return true;
}

void CodeEditor::updateNecData()
{
  codeString = necSubTypeBox->currentText();
  address = necAddressBox->value();
  command = necCommandBox->value();
  auto protocolIndex = necIndexBox->value();
  auto &protocol = ctx.config()->data().getProtocolLib().accessProtocol(
      protocolIndex);
  code = binary::codec::encode(protocolIndex, protocol, codeType,
      codeString.toStdString(), address, command, rawData);
  code.setDelay(static_cast<uint16_t>(necDelayBox->value()));
}

void CodeEditor::updateKasData()
{
  uint64_t data = 0;

  codeString = kasSubTypeBox->currentText();
  address = kasMnfBox->value();
  command = 0;
  data = data | (static_cast<uint64_t>(kasDeviceBox->value()) << 24);
  data = data | (static_cast<uint64_t>(kasSubdeviceBox->value()) << 16);
  data = data | (static_cast<uint64_t>(kasCommandBox->value()) << 8);
  //parity byte not required for encoding
  rawData = binary::irProto::Code::u64bitsToList(48, data);
  auto protocolIndex = kasIndexBox->value();
  auto &protocol = ctx.config()->data().getProtocolLib().accessProtocol(
      protocolIndex);
  code = binary::codec::encode(protocolIndex, protocol, codeType,
      codeString.toStdString(), address, command, rawData);
  code.setRepeats(static_cast<uint8_t>(kasRepeatCountBox->value()));
  code.setDelay(static_cast<uint16_t>(kasDelayBox->value()));
}

void CodeEditor::updateS20Data()
{
  codeString = s20SubTypeBox->currentText();
  address = s20AddressBox->value();
  command = (s20CommandBox->value() & 0x7f)
      | ((s20ExtraBox->value() & 0xff) << 7);

  auto protocolIndex = s20IndexBox->value();
  auto &protocol = ctx.config()->data().getProtocolLib().accessProtocol(
      protocolIndex);
  code = binary::codec::encode(protocolIndex, protocol, codeType,
      codeString.toStdString(), address, command, rawData);
  code.setRepeats(static_cast<uint8_t>(s20RepeatCountBox->value()));
  code.setDelay(static_cast<uint16_t>(s20DelayBox->value()));
}

void CodeEditor::updateACData()
{
  codeString = acSubTypeBox->currentText();
  address = acAddressBox->value();
  command = acCommandBox->value();
  auto protocolIndex = acIndexBox->value();
  auto &protocol = ctx.config()->data().getProtocolLib().accessProtocol(
      protocolIndex);
  code = binary::codec::encode(protocolIndex, protocol, codeType,
      codeString.toStdString(), address, command, rawData);
  code.setRepeats(static_cast<uint8_t>(acRepeatCountBox->value()));
  code.setDelay(static_cast<uint16_t>(acDelayBox->value()));
}

bool CodeEditor::readGroupTable(const QTableWidget *table,
    std::vector<Group> &groups, const QString &name)
{
  for (int row = 0; row < table->rowCount(); row++) {
    auto *symbolsItem = table->item(row, 1);
    auto *bitsItem = table->item(row, 2);
    auto *valueItem = table->item(row, 3);
    if ((symbolsItem == nullptr) || (bitsItem == nullptr)
        || (valueItem == nullptr) || (row >= static_cast<int>(groups.size()))) {
      continue;
    }

    //values that can't be edited are not written back
    if ((valueItem->flags() & Qt::ItemIsEditable) == 0) {
      continue;
    }

    auto symbolCount = static_cast<uint16_t>(symbolsItem->text().toUInt());
    auto bits = static_cast<uint8_t>(bitsItem->text().toUInt());

    std::vector<uint8_t> symbols;
    if (!parseValue(valueItem->text(), symbolCount, bits, symbols)) {
      QMessageBox::warning(this, tr("Edit IR Code"),
          tr("%1, row %2: invalid value \"%3\" for %4 symbols with %5 bit(s)").arg(
              name).arg(row + 1).arg(valueItem->text()).arg(symbolCount).arg(
              bits));
      return false;
    }
    groups[row] = Group(groups[row].getSection(), bits, symbols);
  }
  return true;
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
