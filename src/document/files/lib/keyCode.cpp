// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QRegularExpression>
#include <QStringList>

#include "keyCode.h"

using namespace std;

namespace document
{
namespace files
{

bool KeyCodeNumber::toUInt64(uint64_t &value) const
{
  bool ok = false;
  const uint64_t converted = digits.toULongLong(&ok, 16);
  if (ok) {
    value = converted;
  }
  return ok;
}

bool KeyCode::parse(const QString &text, KeyCode &result)
{
  const QString line = text.trimmed();

  const int groupEnd = line.indexOf(QLatin1Char(':'));
  if (groupEnd < 0) {
    return false;
  }
  const int nameEnd = line.indexOf(QLatin1String(":("), groupEnd + 1);
  if (nameEnd < 0) {
    return false;
  }

  KeyCode parsed;
  parsed.group = line.left(groupEnd);
  parsed.name = line.mid(groupEnd + 1, nameEnd - groupEnd - 1);
  parsed.bitCount = parseBitCount(parsed.name);

  int pos = nameEnd + 1;
  int itemIndex = 0;
  while (itemIndex < 3 && pos < line.size() && line.at(pos) == QLatin1Char('(')) {
    const int close = line.indexOf(QLatin1Char(')'), pos);
    if (close < 0) {
      return false;
    }
    parsed.items.at(itemIndex) = parseItem(line.mid(pos + 1, close - pos - 1));
    itemIndex++;
    pos = close + 1;
    while (pos < line.size() && line.at(pos).isSpace()) {
      pos++;
    }
  }

  if (pos < line.size() && line.at(pos) == QLatin1Char(':')) {
    bool ok = false;
    const int value = line.mid(pos + 1).trimmed().toInt(&ok);
    if (ok) {
      parsed.trailer = value;
    }
  }

  result = parsed;
  return true;
}

QString KeyCode::getGroup() const
{
  return group;
}

QString KeyCode::getName() const
{
  return name;
}

int KeyCode::getBitCount() const
{
  return bitCount;
}

bool KeyCode::isQuadBitType() const
{
  static const QRegularExpression quadRegex(QStringLiteral("\\bQuad\\b"));
  return quadRegex.match(name).hasMatch();
}

int KeyCode::getQuadSymbol(const KeyCodeNumber &number, int index) const
{
  if ((index < 0) || (index >= number.digits.size())) {
    return -1;
  }
  return number.digits.at(index).digitValue();
}

int KeyCode::getTrailer() const
{
  return trailer;
}

QVariant KeyCode::getItem(int index) const
{
  if ((index < 0) || (index > 2)) {
    return QVariant();
  }
  return items.at(index);
}

KeyCode::ItemType KeyCode::getItemType(int index) const
{
  return getItemType(getItem(index));
}

KeyCode::ItemType KeyCode::getItemType(const QVariant &value)
{
  if (!value.isValid()) {
    return ItemType::Empty;
  }
  if (value.userType() == QMetaType::QVariantList) {
    return ItemType::Array;
  }
  if (value.userType() == qMetaTypeId<KeyCodeNumber>()) {
    return ItemType::Number;
  }
  return ItemType::Text;
}

int KeyCode::parseBitCount(const QString &name)
{
  static const QRegularExpression bitRegex(QStringLiteral("(\\d+)\\s*bits?\\b"),
      QRegularExpression::CaseInsensitiveOption);

  const QRegularExpressionMatch match = bitRegex.match(name);
  if (!match.hasMatch()) {
    return -1;
  }
  return match.captured(1).toInt();
}

QVariant KeyCode::parseElement(const QString &token)
{
  static const QRegularExpression numberRegex(
      QStringLiteral("^(\\d*)x([0-9A-Fa-f]+)$"));

  const QRegularExpressionMatch match = numberRegex.match(token);
  if (!match.hasMatch()) {
    // words like "Start" / "Repeat", but also malformed values in the data
    // ("0x0x020122", "0x00FF48B7v", "0700000C"). nothing gets lost this way.
    return QVariant(token);
  }

  KeyCodeNumber number;
  number.prefix = match.captured(1).toInt();
  number.digits = match.captured(2);
  return QVariant::fromValue(number);
}

QVariant KeyCode::parseItem(const QString &content)
{
  if (content.isEmpty()) {
    return QVariant();
  }
  if (!content.contains(QLatin1Char('_'))) {
    return parseElement(content);
  }

  QVariantList list;
  const QStringList tokens = content.split(QLatin1Char('_'));
  for (const QString &token : tokens) {
    list.append(parseElement(token));
  }
  return QVariant(list);
}

}
}

