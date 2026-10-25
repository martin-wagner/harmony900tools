// SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>
#include <cmath>

#include <QDebug>
#include <QFile>
#include <QObject>

#include "lib/qtHelpers.h"
#include "jsonProtocols.h"

using namespace std;
using namespace nlohmann;

namespace document
{
namespace files
{

ProtocolDatabase::ProtocolDatabase()
{
}

bool ProtocolDatabase::setDatabase(const DatabasePaths &paths)
{
  clear();
  errStr = "OK";
  path = paths.protocols;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    return loadFailed(QObject::tr("load: failed to open %1").arg(path));
  }
  const QByteArray content = file.readAll();
  file.close();
  if (content.isEmpty()) {
    return loadFailed(QObject::tr("load: %1 is empty").arg(path));
  }

  try {
    j = json::parse(content.constData(), content.constData() + content.size());

    if (!j.is_object() || !j.contains("Protocols")
        || !j.at("Protocols").is_array()) {
      return loadFailed(QObject::tr("load: no Protocol entries"));
    }
    const json &list = j.at("Protocols");
    if (list.empty()) {
      return loadFailed(QObject::tr("load: protocols is empty"));
    }

    for (size_t i = 0; i < list.size(); i++) {
      const json &jp = list.at(i);
      if (!jp.is_object() || !jp.contains("Id-")
          || !jp.at("Id-").is_number_unsigned()) {
        continue;
      }
      const auto id = jp.at("Id-").get<uint64_t>();
      //0 = RAW, the code index has to fit into 16 bit
      if ((id == 0) || (id >= binary::irProto::Code::INVALID_INDEX_MARKER)) {
        continue;
      }
      if (idToIndex.count(static_cast<uint32_t>(id)) > 0) {
        return loadFailed(QObject::tr("load: Id %1 is used twice").arg(id));
      }
      idToIndex.emplace(static_cast<uint32_t>(id), i);

      const QString name = normalizeName(qstr(jp.value("Name", string())));
      if (name.isEmpty()) {
        return loadFailed(QObject::tr("load: Id %1 name is empty").arg(id));
      }
      if (nameToId.contains(name)) {
        return loadFailed(
            QObject::tr("load: Id %1 name already in use").arg(id));
      }
      nameToId.insert(name, static_cast<uint32_t>(id));
    }
  } catch (const json::exception &e) {
    return loadFailed(
        QObject::tr("load: Json error [%1]: %2").arg(e.id).arg(
            QString::fromUtf8(e.what())));
  }
  return true;
}

binary::irProto::IrProto ProtocolDatabase::getProtocol(uint32_t id,
    string *name)
{
  errStr = "OK";

  const CachedProtocol *cached = loadProtocol(id);
  if (cached == nullptr) {
    return binary::irProto::IrProto();
  }
  if (name != nullptr) {
    *name = cached->name.toStdString();
  }
  return cached->proto;
}

binary::irProto::Code ProtocolDatabase::getCode(const QString &keyCodeLine)
{
  errStr = "OK";

  KeyCode key;
  if (!KeyCode::parse(keyCodeLine, key)) {
    errStr = QObject::tr("keycode: malformed line");
    return binary::irProto::Code();
  }
  if (key.getGroup() != QStringLiteral("G")) {
    errStr = QObject::tr("keycode: raw commands are not supported");
    return binary::irProto::Code();
  }

  const uint32_t id = findId(key.getName());
  if (id == 0) {
    errStr = QObject::tr("keycode: protocol %1 not found").arg(key.getName());
    return binary::irProto::Code();
  }
  const CachedProtocol *cached = loadProtocol(id);
  if (cached == nullptr) {
    //errstr already filled in
    return binary::irProto::Code();
  }

  //every group is a list of frames, each frame is one protocol section
  array<vector<pair<uint8_t, vector<uint8_t>>>, 3> groups;
  for (int g = 0; g < groups.size(); g++) {
    if (!buildGroup(elementsOf(key.getItem(g)), *cached, groups[g])) {
      return binary::irProto::Code();
    }
  }
  if (groups[0].empty() && groups[1].empty() && groups[2].empty()) {
    errStr = QObject::tr("keycode: %1 has no data").arg(key.getName());
    return binary::irProto::Code();
  }

  binary::irProto::Code code(binary::irProto::Code::INVALID_INDEX_MARKER,
      cached->proto); //protocol index in output binary, not source id. Index not known yet.
  code.setStartGroups(groups[0]);
  code.setRepeatGroups(groups[1]);
  code.setFinishGroups(groups[2]);
  if (!code.isValid(true)) {
    errStr = QObject::tr("keycode: could not build a valid code for %1").arg(
        key.getName());
    return binary::irProto::Code();
  }
  return code;
}

void ProtocolDatabase::clear()
{
  j = json();
  idToIndex.clear();
  nameToId.clear();
  irProtoCache.clear();
}

bool ProtocolDatabase::loadFailed(const QString &message)
{
  //no half loaded database
  clear();
  errStr = message;
  return false;
}

QString ProtocolDatabase::normalizeName(const QString &name)
{
  return name.trimmed().toLower();
}

uint32_t ProtocolDatabase::findId(const QString &name) const
{
  //0 = not found, Id- never is 0
  return nameToId.value(normalizeName(name), 0);
}

const ProtocolDatabase::CachedProtocol* ProtocolDatabase::loadProtocol(
    uint32_t id)
{
  if (id == 0) {
    errStr = QObject::tr("ID0 = RAW, not PROTO");
    return nullptr;
  }
  if (idToIndex.empty()) {
    errStr = QObject::tr("load: no database loaded");
    return nullptr;
  }

  const auto cached = irProtoCache.find(id);
  if (cached != irProtoCache.end()) {
    return &cached->second;
  }
  const auto index = idToIndex.find(id);
  if (index == idToIndex.end()) {
    errStr = QObject::tr("load: Protocol[%1] does not exist").arg(id);
    return nullptr;
  }

  CachedProtocol entry;
  try {
    const json &jp = j.at("Protocols").at(index->second);
    entry.name = qstr(jp.value("Name", string()));
    auto res = buildProtocol(jp, entry);
    if (res != true) {
      return nullptr;
    }
  } catch (const json::exception &e) {
    errStr = QObject::tr("Protocol json reader exception: %1").arg(
        QString::fromUtf8(e.what()));
    return nullptr;
  }
  auto ret = irProtoCache.emplace(id, move(entry));
  return &ret.first->second;
}

bool ProtocolDatabase::buildProtocol(const json &jp, CachedProtocol &entry)
{
  //ip and raw only protocols have no key codes
  if (!jp.contains("KeyCode") || !jp.at("KeyCode").is_object()) {
    errStr = QObject::tr(
        "load: protocol has no key codes (IP or raw only, not supported)");
    return false;
  }

  double carrier = binary::irProto::DEFAULT_CLOCK_HZ;
  if (jp.contains("CarrierFrequency")
      && jp.at("CarrierFrequency").is_number()) {
    carrier = jp.at("CarrierFrequency").get<double>();
  }
  if (carrier <= 0.0) {
    errStr = QObject::tr("load: invalid CarrierFrequency");
    return false;
  }
  const long clockPeriod = lround(1000000000.0 / carrier);
  if ((clockPeriod <= 0) || (clockPeriod > 0xffff)) {
    errStr = QObject::tr("load: CarrierFrequency %1 Hz can not be "
        "stored (clock period %2 ns)").arg(carrier).arg(clockPeriod);
    return false;
  }

  //sections: IRSegments first, their index is what the key codes use. then the data-less CodeSegments.
  static const char *listNames[2] = { "IRSegments", "CodeSegments" };
  vector<binary::irProto::TimingSection> sections;

  for (int listIndex = 0; listIndex < 2; listIndex++) {
    if (!jp.contains(listNames[listIndex])) {
      continue;
    }
    const json &list = jp.at(listNames[listIndex]);
    if (!list.is_array()) {
      errStr = QObject::tr("load: %1 is not a list").arg(
          QString::fromUtf8(listNames[listIndex]));
      return false;
    }

    for (const auto &segment : list) {
      if (sections.size() >= MAX_SECTIONS) {
        errStr = QObject::tr("load: too many sections");
        return false;
      }

      binary::irProto::TimingSection section;
      auto res = buildSection(segment, section);
      if (res != true) {
        return false;
      }

      //exception for code segments only -- fetch the keyword for payload-less items
      if (listIndex == 1) {
        const QString keyword = getKeyword(entry.name,
            qstr(segment.value("Name", string())));
        if ((!keyword.isEmpty()) && (!entry.keywords.contains(keyword))) {
          entry.keywords.insert(keyword, static_cast<uint8_t>(sections.size()));
        }
      }
      sections.push_back(section);
    }
  }

  if (sections.empty()) {
    errStr = QObject::tr("load: protocol has no sections");
    return false;
  }

  entry.proto = binary::irProto::IrProto(static_cast<uint16_t>(clockPeriod),
      sections);
  return true;
}

QString ProtocolDatabase::getKeyword(const QString &protocolName,
    const QString &segmentName)
{
  static const QString marker = QStringLiteral("keycode");

  const QString prefix = normalizeName(protocolName);
  const QString name = normalizeName(segmentName);
  if (!name.startsWith(prefix)) {
    return QString();
  }

  const QString rest = name.mid(prefix.size()).trimmed();
  if (!rest.startsWith(marker)) {
    return QString();
  }
  return rest.mid(marker.size()).trimmed();
}

QVariantList ProtocolDatabase::elementsOf(const QVariant &item)
{
  QVariantList elements;
  const KeyCode::ItemType type = KeyCode::getItemType(item);
  if (type == KeyCode::ItemType::Array) {
    elements = item.toList();
  }
  if ((type == KeyCode::ItemType::Number)
      || (type == KeyCode::ItemType::Text)) {
    elements.append(item);
  }
  return elements;
}

void ProtocolDatabase::appendItems(const json &atom,
    vector<binary::irProto::Item> &out)
{
  const bool state = (atom.at("Type").get<int>() == 1);
  uint32_t remaining = atom.at("Value").get<uint32_t>();

  //too long for one item: max-length item, then 0 us of the other state, repeat
  while (remaining > MAX_SINGLE_TIMING) {
    out.emplace_back(state, static_cast<uint16_t>(MAX_SINGLE_TIMING));
    out.emplace_back(!state, static_cast<uint16_t>(0));
    remaining -= MAX_SINGLE_TIMING;
  }
  out.emplace_back(state, static_cast<uint16_t>(remaining));
}

bool ProtocolDatabase::buildHeader(const json &atoms,
    binary::irProto::TimingSectionIrHeader &header)
{
  vector<binary::irProto::Item> items;
  for (const auto &atom : atoms) {
    appendItems(atom, items);
  }
  if (items.size() > MAX_ITEMS) {
    errStr = QObject::tr(
        "load: header / trailer needs %1 timing items, max is %2").arg(
        items.size()).arg(MAX_ITEMS);
    return false;
  }

  for (const auto &item : items) {
    header.addItem(item);
  }
  return true;
}

bool ProtocolDatabase::buildSection(const json &segment,
    binary::irProto::TimingSection &section)
{
  using Quantum = binary::irProto::TimingSectionIrPayload::Quantum;

  //interval is the total frame time. if not given, the pause is coded in the eof (default)
  const json total = segment.value("TotalLength", json());
  if (total.is_number_unsigned() && (total.get<uint32_t>() > 0)) {
    section.setTiming(total.get<uint32_t>());
  }

  const json payload = segment.value("Payload", json());
  if (payload.is_object()) {
    section.setBitCount(payload.at("NumberOfBits").get<uint16_t>());

    const json toggleBit = payload.value("ToggleBit", json());
    if (toggleBit.is_number_unsigned()) {
      section.setToggle(toggleBit.get<uint16_t>());
    }

    //the symbol is the "BitType", not the position in the list
    const json &encodings = payload.at("Encodings");
    const size_t quantumCount = encodings.size();
    vector<Quantum> quanta(quantumCount);
    vector<bool> assigned(quantumCount, false);
    size_t maxItems = 0;

    for (const auto &encoding : encodings) {
      const auto bitType = encoding.at("BitType").get<size_t>();
      if ((bitType >= quantumCount) || assigned[bitType]) {
        errStr = QObject::tr("load: invalid BitType %1").arg(bitType);
        return false;
      }
      assigned[bitType] = true;

      for (const auto &atom : encoding.at("Atoms")) {
        appendItems(atom, quanta[bitType]);
      }
      maxItems = max(maxItems, quanta[bitType].size());
    }
    if (maxItems > MAX_ITEMS) {
      errStr = QObject::tr("load: encoding needs %1 timing "
          "items, max is %2").arg(maxItems).arg(MAX_ITEMS);
      return false;
    }

    //all quanta need the same item count: pad with 0 us items, alternating state
    binary::irProto::TimingSectionIrPayload data;
    for (size_t q = 0; q < quantumCount; q++) {
      Quantum &quantum = quanta[q];
      bool state = true;
      if (!quantum.empty()) {
        state = !quantum.back().first;
      }
      while (quantum.size() < maxItems) {
        quantum.emplace_back(state, static_cast<uint16_t>(0));
        state = !state;
      }
      if (!data.setQuantum(static_cast<uint8_t>(q), quantum)) {
        errStr = QObject::tr("load: invalid encoding %1").arg(q);
        return false;
      }
    }
    section.setData(data);
  }

  binary::irProto::TimingSectionIrHeader sof;
  if (!buildHeader(segment.value("Header", json::array()), sof)) {
    return false;
  }
  section.setSoF(sof);

  binary::irProto::TimingSectionIrHeader eof;
  if (!buildHeader(segment.value("Trailer", json::array()), eof)) {
    return false;
  }
  section.setEoF(eof);

  return true;
}

bool ProtocolDatabase::buildGroup(const QVariantList &elements,
    const CachedProtocol &protocol, vector<pair<uint8_t, vector<uint8_t>>> &out)
{
  //every element is one frame, in the given order. a section can be left out or used multiple times.
  for (const QVariant &element : elements) {
    const KeyCode::ItemType type = KeyCode::getItemType(element);

    if (type == KeyCode::ItemType::Number) {
      //section with data. the number in front of the "x" is the protocol section
      const KeyCodeNumber number = element.value<KeyCodeNumber>();
      if ((number.prefix < 0)
          || (number.prefix >= protocol.proto.getSectionCount())) {
        errStr = QObject::tr("keycode: section %1 does not exist").arg(
            number.prefix);
        return false;
      }

      vector<uint8_t> symbols;
      if (!buildSymbols(number, protocol.proto.accessSection(number.prefix),
          symbols)) {
        return false;
      }
      out.emplace_back(static_cast<uint8_t>(number.prefix), symbols);
    }

    if (type == KeyCode::ItemType::Text) {
      //word: section without data
      const QString word = normalizeName(element.toString());
      if (word == QStringLiteral("key")) {
        errStr = QObject::tr("keycode: key codes (IP) are not supported");
        return false;
      }

      const auto section = protocol.keywords.constFind(word);
      if (section == protocol.keywords.constEnd()) {
        errStr = QObject::tr("keycode: unknown word \"%1\"").arg(word);
        return false;
      }
      out.emplace_back(section.value(), vector<uint8_t>());
    }
  }
  return true;
}

bool ProtocolDatabase::buildSymbols(const KeyCodeNumber &number,
    const binary::irProto::TimingSection &section, vector<uint8_t> &symbols)
{
  const int bitCount = section.getBitCount();
  const int quantums = section.getData().getQuantisation();
  const int digitCount = static_cast<int>(number.digits.size());
  symbols.clear();

  //value as symbol list, msb first
  vector<uint8_t> value;
  if (quantums == 2) {
    //bool: every hex digit is 4 bits
    for (int i = 0; i < digitCount; i++) {
      const int digit = KeyCode::getHexSymbol(number, i);
      if (digit < 0) {
        errStr = QObject::tr("keycode: invalid digit in %1").arg(number.digits);
        return false;
      }
      for (int bit = 3; bit >= 0; bit--) {
        value.push_back(static_cast<uint8_t>((digit >> bit) & 1));
      }
    }
  } else if ((quantums == 4) || (quantums == 16)) {
    //quad / hex: every digit is one symbol
    for (int i = 0; i < digitCount; i++) {
      int symbol = -1;
      if (quantums == 4) {
        symbol = KeyCode::getQuadSymbol(number, i);
      } else {
        symbol = KeyCode::getHexSymbol(number, i);
      }
      if (symbol < 0) {
        errStr = QObject::tr("keycode: invalid symbol in %1").arg(
            number.digits);
        return false;
      }
      value.push_back(static_cast<uint8_t>(symbol));
    }
  } else {
    errStr = QObject::tr("keycode: section %1 takes no data").arg(
        number.prefix);
    return false;
  }

  //leading zeros are not needed in the key code, the section defines the width
  size_t first = 0;
  while ((first < value.size()) && (value[first] == 0)) {
    first++;
  }
  const size_t significant = value.size() - first;
  if (significant > static_cast<size_t>(bitCount)) {
    errStr = QObject::tr("keycode: value %1 does not fit into %2 symbols").arg(
        number.digits).arg(bitCount);
    return false;
  }

  symbols.assign(static_cast<size_t>(bitCount) - significant, 0);
  symbols.insert(symbols.end(), value.begin() + static_cast<ptrdiff_t>(first),
      value.end());
  return true;
}

}
}
