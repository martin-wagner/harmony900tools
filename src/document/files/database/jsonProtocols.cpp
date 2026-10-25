// SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>
#include <cmath>
#include <limits>

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

  //every group is a list of frames, each frame uses one protocol section
  array<vector<pair<uint8_t, vector<uint8_t>>>, 3> groups;
  for (int g = 0; g < groups.size(); g++) {
    if (!buildGroup(key.getItem(g), *cached, groups[g])) {
      return binary::irProto::Code();
    }
  }
  if (groups[0].empty() && groups[1].empty() && groups[2].empty()) {
    errStr = QObject::tr("keycode: %1 has no data").arg(key.getName());
    return binary::irProto::Code();
  }

  binary::irProto::Code code(binary::irProto::Code::INVALID_INDEX_MARKER,
      cached->proto); //protocol index in output binary, not id! Index not known yet.
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
  CachedProtocol entry;

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
  vector<binary::irProto::TimingSection> sections;

  //ip and raw only protocols have no key codes
  if (!jp.contains("KeyCode") || !jp.at("KeyCode").is_object()) {
    errStr = QObject::tr(
        "load: protocol has no key codes (IP or raw only, not supported)");
    return false;
  }

  auto carrier = binary::irProto::DEFAULT_CLOCK_HZ;
  if (jp.contains("CarrierFrequency")
      && jp.at("CarrierFrequency").is_number()) {
    carrier = jp.at("CarrierFrequency").get<double>();
  }
  if (carrier <= 0.0) {
    errStr = QObject::tr("load: invalid CarrierFrequency");
    return false;
  }
  auto clockPeriod = lround(1000000000.0 / carrier);
  if ((clockPeriod <= 0) || (clockPeriod > 0xffff)) {
    errStr = QObject::tr("load: CarrierFrequency %1 Hz can not be "
        "stored (clock period %2 ns)").arg(carrier).arg(clockPeriod);
    return false;
  }

  //we have two options here:
  // - read KeyCode object, use the Start/Repeat/Finish items, and use the "pointers"
  //   within to build the sections.
  // - read IRSegments / CodeSegments arrays to sections directly, ignoring KeyCode.
  //   This follows the order in the arrays.
  //The direct approach always returns valid data, also in combination with getCode()
  //further on. The indirect approach is not reliable. Maybe KeyCode was never used?

  //sections: IRSegments first, their index is what the key codes use. then the data-less CodeSegments.
  static const char *listNames[2] = { "IRSegments", "CodeSegments" };

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
      binary::irProto::TimingSection section;

      if (sections.size() >= MAX_SECTIONS) {
        errStr = QObject::tr("load: too many sections");
        return false;
      }

      auto res = buildSection(segment, section);
      if (res != true) {
        return false;
      }

      //for code segments only -- fetch the keyword "KeyCode>>X<<"
      if (listIndex == 1) {
        auto name = qstr(segment.value("Name", string()));
        auto keyword = getKeyword(entry.name, name);
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
  if (entry.proto.serialise(0).size() > MAX_PROTOCOL_SIZE) {
    //plausibility only -- all protocols in the remote share the same space
    errStr = QObject::tr("load: protocol is too big for IrProto.bin");
    return false;
  }
  return true;
}

QString ProtocolDatabase::getKeyword(const QString &protocolName,
    const QString &segmentName)
{
  static const auto marker = QStringLiteral("keycode");

  auto prefix = normalizeName(protocolName);
  auto name = normalizeName(segmentName);
  if (!name.startsWith(prefix)) {
    return QString();
  }

  auto rest = name.mid(prefix.size()).trimmed();
  if (!rest.startsWith(marker)) {
    return QString();
  }
  return rest.mid(marker.size()).trimmed();
}

bool ProtocolDatabase::appendItems(const json &atom,
    std::vector<binary::irProto::Item> &out)
{
  //everything has to fit into the IrProto format, there is no "almost"
  const json &type = atom.at("Type");
  if (!type.is_number_unsigned() || (type.get<uint64_t>() > 1)) {
    errStr = QObject::tr("load: timing type has to be 0 (pause) or 1 (mark)");
    return false;
  }
  const json &value = atom.at("Value");
  if (!value.is_number_unsigned()
      || (value.get<uint64_t>() > std::numeric_limits<uint32_t>::max())) {
    errStr = QObject::tr("load: invalid timing value");
    return false;
  }

  const bool state = (type.get<uint64_t>() == 1);
  auto remaining = value.get<uint32_t>();

  //a pause that long is a database error. keep the first part, drop the rest.
  if (!state && (remaining > MAX_PAUSE)) {
    qDebug() << "pause limited to" << MAX_PAUSE << "us, database has"
        << remaining << "us";
    remaining = MAX_PAUSE;
  }

  //too long for one item: max-length item, then 0 us of the other state, repeat
  size_t needed = 1;
  if (remaining > 0) {
    needed = 1 + 2 * static_cast<size_t>((remaining - 1) / MAX_SINGLE_TIMING);
  }
  if ((out.size() + needed) > MAX_ITEMS) {
    errStr = QObject::tr("load: more than %1 timing items").arg(MAX_ITEMS);
    return false;
  }
  while (remaining > MAX_SINGLE_TIMING) {
    out.emplace_back(state, static_cast<uint16_t>(MAX_SINGLE_TIMING));
    out.emplace_back(!state, static_cast<uint16_t>(0));
    remaining -= MAX_SINGLE_TIMING;
  }
  out.emplace_back(state, static_cast<uint16_t>(remaining));
  return true;
}

bool ProtocolDatabase::buildHeader(const json &atoms,
    binary::irProto::TimingSectionIrHeader &header)
{
  std::vector<binary::irProto::Item> items;
  for (const auto &atom : atoms) {
    if (!appendItems(atom, items)) {
      return false;
    }
  }

  for (const auto &item : items) {
    header.addItem(item);
  }
  return true;
}

bool ProtocolDatabase::buildPayload(const json &payload,
    binary::irProto::TimingSection &section)
{
  using Quantum = binary::irProto::TimingSectionIrPayload::Quantum;

  const json &bitCount = payload.at("NumberOfBits");
  if (!bitCount.is_number_unsigned()
      || (bitCount.get<uint64_t>() > MAX_BIT_COUNT)) {
    errStr = QObject::tr("load: invalid NumberOfBits");
    return false;
  }
  section.setBitCount(bitCount.get<uint16_t>());

  //null = no toggle. the last position is the "none" marker of the format.
  const json toggleBit = payload.value("ToggleBit", json());
  if (!toggleBit.is_null()) {
    if (!toggleBit.is_number_unsigned()
        || (toggleBit.get<uint64_t>()
            >= binary::irProto::TimingSection::TOGGLE_NONE)) {
      errStr = QObject::tr("load: invalid ToggleBit");
      return false;
    }
    section.setToggle(toggleBit.get<uint16_t>());
  }

  //the symbol is the "BitType", not the position in the list
  const json &encodings = payload.at("Encodings");
  auto quantumCount = encodings.size();
  if (quantumCount > MAX_SYMBOLS) {
    errStr = QObject::tr("load: more than %1 symbols").arg(MAX_SYMBOLS);
    return false;
  }
  if ((quantumCount == 0) && (bitCount.get<uint64_t>() > 0)) {
    errStr = QObject::tr("load: payload has bits but no symbols");
    return false;
  }

  std::vector<Quantum> quanta(quantumCount);
  std::vector<bool> assigned(quantumCount, false);
  size_t maxItems = 0;

  for (const auto &encoding : encodings) {
    auto bitType = encoding.at("BitType").get<size_t>();
    if ((bitType >= quantumCount) || assigned[bitType]) {
      errStr = QObject::tr("load: invalid BitType %1").arg(bitType);
      return false;
    }
    assigned[bitType] = true;

    for (const auto &atom : encoding.at("Atoms")) {
      if (!appendItems(atom, quanta[bitType])) {
        return false;
      }
    }
    maxItems = std::max(maxItems, quanta[bitType].size());
  }

  //all quanta need the same item count: pad with 0 us items, alternating state
  binary::irProto::TimingSectionIrPayload data;
  for (size_t q = 0; q < quantumCount; q++) {
    auto &quantum = quanta[q];
    auto state = true;
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
  return true;
}

bool ProtocolDatabase::buildSection(const json &segment,
    binary::irProto::TimingSection &section)
{
  //interval is the total frame time. if not given, the pause is coded in the eof (default)
  const json total = segment.value("TotalLength", json());
  if (!total.is_null()) {
    //the last value is the "pause in eof" marker of the format
    if (!total.is_number_unsigned()
        || (total.get<uint64_t>()
            >= binary::irProto::TimingSection::PAUSE_IN_EOF)) {
      errStr = QObject::tr("load: invalid TotalLength");
      return false;
    }
    if (total.get<uint32_t>() > 0) {
      section.setTiming(total.get<uint32_t>());
    }
  }

  //code segments have no payload
  const json payload = segment.value("Payload", json());
  if (payload.is_object() && !buildPayload(payload, section)) {
    return false;
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

bool ProtocolDatabase::buildFrame(const QVariant &element,
    const CachedProtocol &protocol,
    std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &out)
{
  std::vector<uint8_t> symbols;
  auto type = KeyCode::getItemType(element);

  if (type == KeyCode::ItemType::Number) {
    //section with data. the number in front of the "x" is the protocol section
    auto number = element.value<KeyCodeNumber>();
    if ((number.prefix < 0)
        || (number.prefix >= protocol.proto.getSectionCount())) {
      errStr = QObject::tr("keycode: section %1 does not exist").arg(
          number.prefix);
      return false;
    }

    if (!buildSymbols(number, protocol.proto.accessSection(number.prefix),
        symbols)) {
      return false;
    }
    out.emplace_back(static_cast<uint8_t>(number.prefix), symbols);
    return true;
  }

  if (type == KeyCode::ItemType::Text) {
    //word: section without data
    auto word = normalizeName(element.toString());
    if (word == QStringLiteral("key")) {
      errStr = QObject::tr("keycode: key codes (IP) are not supported");
      return false;
    }

    const auto section = protocol.keywords.constFind(word);
    if (section == protocol.keywords.constEnd()) {
      errStr = QObject::tr("keycode: unknown word \"%1\"").arg(word);
      return false;
    }
    out.emplace_back(section.value(), std::vector<uint8_t>());
    return true;
  }

  errStr = QObject::tr("keycode: unexpected element");
  return false;
}

bool ProtocolDatabase::buildGroup(const QVariant &item,
    const CachedProtocol &protocol,
    std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &out)
{
  //every element is one frame, in the given order. a section can be left out or used multiple times.
  switch (KeyCode::getItemType(item)) {
    case KeyCode::ItemType::Empty:
      //"()": nothing to send in this group
      return true;
    case KeyCode::ItemType::Array:
      //"a_b_c": a list of frames
      for (const auto &element : item.toList()) {
        if (!buildFrame(element, protocol, out)) {
          return false;
        }
      }
      return true;
    case KeyCode::ItemType::Number:
    case KeyCode::ItemType::Text:
      //single frame
      return buildFrame(item, protocol, out);
  }
  return false;
}

bool ProtocolDatabase::buildSymbols(const KeyCodeNumber &number,
    const binary::irProto::TimingSection &section, vector<uint8_t> &symbols)
{
  vector<uint8_t> value;

  auto bitCount = section.getBitCount();
  auto quantums = section.getData().getQuantisation();
  auto digitCount = static_cast<int>(number.digits.size());
  symbols.clear();

  //value as symbol list, msb first
  if (quantums == 2) {
    //bool: every hex digit is 4 bits
    for (int i = 0; i < digitCount; i++) {
      auto digit = KeyCode::getHexSymbol(number, i);
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
  auto significant = value.size() - first;
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
