// SPDX-License-Identifier: LGPL-2.1-or-later

#include <fstream>

#include "file.h"

#include "lib/endian.h"
#include "lib/crc32.h"

using namespace std;

namespace irProto
{

TimingSectionIrHeader::TimingSectionIrHeader()
{
}

TimingSectionIrHeader::TimingSectionIrHeader(vector<uint8_t> data)
{
  if (data.empty()) {
    return;
  }

  auto itemCount = data[0];
  data.erase(data.begin(), data.begin() + 1);

  if (((data.size() % 2) != 0) || (data.size()) < (itemCount * 2)) {
    return;
  }

  //data is "mark" when MSB is set
  for (int i = 0; i < data.size(); i += 2) {
    auto word = lib::parseHarmony16_file(data[i], data[i + 1]);
    if ((word & 0x8000) != 0) {
      stream.push_back( { true, word & 0x7fff });
    } else {
      stream.push_back( { false, word });
    }
  }
}

void TimingSectionIrHeader::addItem(Item item)
{
  if ((item.second & 0x8000) != 0) {
    //crop
    item.second = 0x7fff;
  }

  stream.push_back(item);
}

vector<uint8_t> TimingSectionIrHeader::serialise() const
{
  vector<uint8_t> data;

  if (stream.empty()) {
    return data;
  }

  //size
  data.push_back(stream.size());
  //data
  for (const auto &item : stream) {
    if (item.first == true) {
      lib::setHarmony16_file(item.second | 0x8000, data);
    } else {
      lib::setHarmony16_file(item.second, data);
    }
  }
  return data;
}

void TimingSectionIrHeader::serialiseIrStream(vector<Item> &out) const
{
  out.insert(out.end(), stream.begin(), stream.end());
}

TimingSectionIrPayload::TimingSectionIrPayload()
{
}

TimingSectionIrPayload::TimingSectionIrPayload(const vector<uint8_t> &data)
{
  vector<uint16_t> tmp;
  vector<Item> stream;

  if (data.empty()) {
    return;
  }

  if ((data.size() % 2) != 0) {
    return;
  }

  auto ret = lib::parseHarmony16_file(data, tmp);
  if ((ret != true) || (tmp.size() < 2)) {
    return;
  }

  for (const auto &word : tmp) {
    if ((word & 0x8000) != 0) {
      stream.push_back( { true, word & 0x7fff });
    } else {
      stream.push_back( { false, word });
    }
  }

  if (stream.size() == 2) {
    streamFalse.push_back(stream[0]); //todo this needs to be coded as true or false??
    streamFalse.push_back(stream[1]);
  } else if (stream.size() == 4) {
    streamFalse.push_back(stream[0]);
    streamFalse.push_back(stream[1]);
    streamTrue.push_back(stream[2]);
    streamTrue.push_back(stream[3]);
  }
}

void TimingSectionIrPayload::setFalsePair(vector<Item> timings)
{
  if (timings.size() != 2) {
    streamFalse.clear();
    return;
  }
  streamFalse = timings;
}

void TimingSectionIrPayload::setTruePair(vector<Item> timings)
{
  if (timings.size() != 2) {
    streamTrue.clear();
    return;
  }
  streamTrue = timings;
}

vector<uint8_t> TimingSectionIrPayload::serialise() const
{
  vector<uint8_t> data;

  for (const auto &item : streamFalse) {
    if (item.first == true) {
      lib::setHarmony16_file(item.second | 0x8000, data);
    } else {
      lib::setHarmony16_file(item.second, data);
    }
  }
  for (const auto &item : streamTrue) {
    if (item.first == true) {
      lib::setHarmony16_file(item.second | 0x8000, data);
    } else {
      lib::setHarmony16_file(item.second, data);
    }
  }
  return data;
}

void TimingSectionIrPayload::serialiseIrStream(bool booleanState,
    vector<Item> &out) const
{
  if (booleanState == true) {
    out.insert(out.end(), streamTrue.begin(), streamTrue.end());
  } else {
    out.insert(out.end(), streamFalse.begin(), streamFalse.end());
  }
}

TimingSection::TimingSection()
{
}

TimingSection::TimingSection(const vector<uint8_t> &data, int offset)
{
  int pSoF;
  int pData;
  int pEoF;

  if (data.size() < HEADER_SIZE) {
    cout << "Warning: IrProto.bin timing section too small (" << data.size()
        << " bytes)" << endl;
    return;
  }

  bitCount = lib::parseHarmony16_file(data[0], data[1]);
  if (bitCount > 250) {
    cout << "Warning: IrProto.bin timing bit count is very large (" << bitCount
        << " bits)" << endl;
    //ignore
  }
  if (data[3] != 0xff) {
    cout << "Warning: IrProto.bin padding before toggle used (" << (int) data[2]
        << ")" << endl;
  }
  togglePos = data[2];
  interval = lib::parseHarmony32_file(data[4], data[5], data[6], data[7]);
  ctrl0 = static_cast<Ctrl0>(data[8]);
  switch (ctrl0) {
    case Ctrl0::IS_DATA_FRAME:
    case Ctrl0::IS_REPEAT_FRAME:
      break;
    default:
      cout << "Warning: IrProto.bin timing ctrl0 unknown value (0x" << hex
          << (int) ctrl0 << dec << ")" << endl;
      break;
  }
  ctrl1 = static_cast<Ctrl1>(data[9]);
  switch (ctrl1) {
    case Ctrl1::NO_DATA:
    case Ctrl1::DATA_ONE_PAIR:
    case Ctrl1::DATA_TWO_PAIRS:
      break;
    default:
      cout << "Warning: IrProto.bin timing ctrl1 unknown value (0x" << hex
          << (int) ctrl1 << dec << ")" << endl;
      break;
  }

  pData = (int) lib::parseHarmony16_file(data[10], data[11]) - offset;
  pSoF = (int) lib::parseHarmony16_file(data[12], data[13]) - offset;
  pEoF = (int) lib::parseHarmony16_file(data[14], data[15]) - offset;

  if ((pSoF >= HEADER_SIZE) && (pSoF <= data.size())) {
    auto size = data[pSoF] * 2 + 1;
    if ((pSoF + size) <= data.size()) {
      sof = TimingSectionIrHeader(
          { data.begin() + pSoF, data.begin() + pSoF + size });
    }
  }
  if ((pEoF >= HEADER_SIZE) && (pEoF <= data.size())) {
    auto size = data[pEoF] * 2 + 1;
    if ((pEoF + size) <= data.size()) {
      eof = TimingSectionIrHeader(
          { data.begin() + pEoF, data.begin() + pEoF + size });
    }
  }
  if ((pData >= HEADER_SIZE) && (pData <= data.size())) {
    auto size = static_cast<int>(ctrl1) * 4;
    if ((pData + size) <= data.size()) {
      this->data = TimingSectionIrPayload(
          { data.begin() + pData, data.begin() + pData + size });
    }
  }
}

void TimingSection::setBitCount(uint16_t count)
{
  bitCount = count;
}

void TimingSection::setToggle(uint16_t toggle)
{
  togglePos = toggle;
}

void TimingSection::setTiming(uint32_t t_us)
{
  interval = t_us;
}

void TimingSection::setCtrl0(Ctrl0 c)
{
  ctrl0 = c;
}

void TimingSection::setCtrl1(Ctrl1 c)
{
  ctrl1 = c;
}

void TimingSection::setSoF(const TimingSectionIrHeader &sof)
{
  this->sof = sof;
}

void TimingSection::setData(const TimingSectionIrPayload &d)
{
  data = d;
}

void TimingSection::setEoF(const TimingSectionIrHeader &eof)
{
  this->eof = eof;
}

vector<uint8_t> TimingSection::serialise(int offset) const
{
  vector<uint8_t> data;

  lib::setHarmony16_file(bitCount, data);
  data.push_back(togglePos);
  data.push_back(0xff);
  lib::setHarmony32_file(interval, data);
  data.push_back(static_cast<uint8_t>(ctrl0));
  data.push_back(static_cast<uint8_t>(ctrl1));

  auto sofTimings = sof.serialise();
  auto dataTimings = this->data.serialise();
  auto eofTimings = eof.serialise();

  //we keep the original relationship between data pointer position and data position.
  //if the firmware actually interprets this correctly, it shouldn't matter...
  //calc/append pointers
  if (dataTimings.empty()) {
    lib::setHarmony16_file(0, data);
  } else {
    lib::setHarmony16_file(
        offset + HEADER_SIZE + eofTimings.size() + sofTimings.size(), data);
  }
  if (sofTimings.empty()) {
    lib::setHarmony16_file(0, data);
  } else {
    lib::setHarmony16_file(offset + HEADER_SIZE, data);
  }
  if (eofTimings.empty()) {
    lib::setHarmony16_file(0, data);
  } else {
    lib::setHarmony16_file(offset + HEADER_SIZE + sofTimings.size(), data);
  }
  //append data
  data.insert(data.end(), sofTimings.begin(), sofTimings.end());
  data.insert(data.end(), eofTimings.begin(), eofTimings.end());
  data.insert(data.end(), dataTimings.begin(), dataTimings.end());
  return data;
}

void TimingSection::serialiseIrStream(vector<Item> &out,
    const vector<bool> &data) const
{
  vector<Item> tmp;
  int tmsg = 0;

  if (data.size() < bitCount) {
    return;
  }

  sof.serialiseIrStream(tmp);
  for (auto i = 0; i < bitCount; i++) {
    this->data.serialiseIrStream(data[i], tmp);
  }
  eof.serialiseIrStream(tmp);

  for (const auto &item : tmp) {
    tmsg = tmsg + item.second;
  }
  out.insert(out.end(), tmp.begin(), tmp.end());

  auto tinterval = static_cast<int64_t>(interval);
  if ((tinterval != TimingSection::PAUSE_IN_EOF) && (tinterval > 0)) {
    auto tpause = tinterval - tmsg;
    if (tpause <= 0) {
      //oops
      cout << "Warning: IrProto.bin create IR stream negative pause" << endl;
      tpause = 50000; //µs
    }
    do {
      if (tpause > 0x7fff) {
        out.push_back( { false, 0x7fff });
      } else {
        out.push_back( { false, tpause });
      }
      tpause = tpause - 0x7fff;
    } while (tpause > 0);
  }
}

IrProto::IrProto()
{
}

IrProto::IrProto(const vector<uint8_t> &data, int offset)
{
  vector<uint16_t> offsetTable;

  if (data.size() < HEADER_SIZE) {
    cout << "Warning: IrProto.bin protocol section too small (" << data.size()
        << " bytes)" << endl;
    return;
  }
  if ((data[0] != START) || (data[3] != UNUSED) || (data[4] != UNUSED)
      || (data[5] != SEPARATOR)) {
    cout << "Warning: IrProto.bin protocol section header invalid" << endl;
    return;
  }
  //again, include pointers
  auto timingSectionCount = data[6];
  if (data.size() < (HEADER_SIZE + 2 * timingSectionCount)) {
    cout << "Warning: IrProto.bin protocol section too small (pointers, "
        << data.size() << " bytes)" << endl;
    return;
  }

  clockPeriod = lib::parseHarmony16_file(data[1], data[2]);

  lib::parseHarmony16_file(
      { data.begin() + HEADER_SIZE, data.begin() + HEADER_SIZE
          + 2 * timingSectionCount }, offsetTable);

  //offsets need to be within data
  for (auto &o : offsetTable) {
    o = o - offset;
    if ((o) >= data.size()) {
      return;
    }
  }

  for (int i = 0; i < offsetTable.size(); i++) {
    auto start = offsetTable[i];
    auto end = data.size();
    if ((i + 1) < offsetTable.size()) {
      end = offsetTable[i + 1];
    }

    auto s = TimingSection( { data.begin() + start, data.begin() + end },
        offset + start);
    sections.push_back(move(s));
  }
}

const TimingSection& IrProto::accessSection(int index) const
{
  static const TimingSection s;

  if (index >= sections.size()) {
    return s;
  }
  return sections[index];
}

vector<uint8_t> IrProto::serialise(int offset) const
{
  vector<uint8_t> data;
  vector<vector<uint8_t>> serialisedSections;

  data.push_back(START);
  lib::setHarmony16_file(clockPeriod, data);
  data.push_back(UNUSED);
  data.push_back(UNUSED);
  data.push_back(SEPARATOR);
  data.push_back(sections.size());

  offset = offset + HEADER_SIZE + 2 * sections.size();
  for (const auto &s : sections) {
    lib::setHarmony16_file(offset, data);  //append section start offset

    auto section = s.serialise(offset);
    offset = offset + section.size();

    serialisedSections.push_back(section);
  }

  //append sections
  for (const auto &s : serialisedSections) {
    data.insert(data.end(), s.begin(), s.end());
  }

  return data;
}

void IrProto::serialiseIrStream(vector<Item> &out, const Data &data) const
{
  for (const auto &s : data) {
    auto index = s.first;
    if (index >= sections.size()) {
      continue;
    }
    sections[index].serialiseIrStream(out, s.second);
  }
}

vector<Item> File::compress(vector<Item> items)
{
  vector<Item> ret;

  //merge when mutliple timings have the same logic level
  while (items.size() > 0) {
    if (items.size() > 1) {
      auto mark = items[0].first;
      auto time_us = items[0].second;
      auto next_mark = items[1].first;
      auto next_time_us = items[1].second;

      if ((mark == next_mark)) {
        auto compressed = static_cast<uint32_t>(time_us) + next_time_us;
        if (compressed > 0x7fff) {
          items[1].second = compressed - 0x7fff;
          ret.push_back( { mark, 0x7fff });
        } else {
          //add to next timing
          items[1].second = compressed;
        }
      } else {
        ret.push_back(items[0]);
      }
    } else {
      //last item
      ret.push_back(items[0]);
    }
    items.erase(items.begin());
  }
  return ret;
}

File::File()
{
}

File::File(const string &filename)
{
  parse(filename);
}

Status File::parse(const string &filename)
{
  vector<uint16_t> offsetTable;

  protocols.clear();

  ifstream file(filename, ios::binary);
  if (!file.is_open()) {
    return Status::ERROR_FILE;
  }

  auto raw = vector<uint8_t>(istreambuf_iterator<char>(file),
      istreambuf_iterator<char>());

  /* check wrapper */

  //minimum file size
  if (raw.size() < MIN_FILE_SIZE) {
    return Status::ERROR_SIZE;
  }
  auto expectedFileSize = lib::parseHarmony16_file(raw[4], raw[5]);
  if (expectedFileSize != (raw.size() - WRAPPER_SIZE)) {
    return Status::ERROR_OUTER_SIZE;
  }
  //empty bytes
  auto empty = lib::parseHarmony16_file(raw[6], raw[7]);
  if (empty != 0) {
    return Status::ERROR_OUTER_FILE_FORMAT;
  }
  //crc
  auto crc32 = lib::calcCrc32( { raw.begin() + WRAPPER_SIZE, raw.end() });
  auto expectedCrc32 = lib::parseHarmony32_file(raw[0], raw[1], raw[2], raw[3]);
  if (crc32 != expectedCrc32) {
    cout << "Error: IrProto.bin CRC mismatch, file: " << hex << expectedCrc32
        << dec << endl;
    return Status::ERROR_OUTER_CRC;
  }
  raw.erase(raw.begin(), raw.begin() + WRAPPER_SIZE); //makes offset calculations easier

  /* actual file */

  //check header (equal)
  for (int i = 0; i < header.size(); i++) {
    if (header[i] != raw[i]) {
      cout << "Error: IrProto.bin header mismatch at " << i + WRAPPER_SIZE
          << " (is " << raw[i] << ", expected " << header[i] << ")" << endl;
      return Status::ERROR_FILE_FORMAT;
    }
  }
  raw.erase(raw.begin(), raw.begin() + HEADER_SIZE); //makes offset calculations easier

  auto objectCount = lib::parseHarmony16_file(raw[1], raw[2]);
  if (objectCount == 0) {
    //empty file
    return Status::OK;
  }

  auto ret = lib::parseHarmony16_file(
      { raw.begin() + 3, raw.begin() + 3 + 2 * objectCount }, offsetTable);
  if (ret != true) {
    return Status::ERROR_FILE_FORMAT;
  }
  //offsets need to be within the file
  for (const auto &o : offsetTable) {
    if (o > raw.size()) {
      return Status::ERROR_SIZE;
    }
  }

  for (int i = 0; i < objectCount; i++) {
    int start = offsetTable[i];
    int end = raw.size();
    if ((i + 1) < offsetTable.size()) {
      end = offsetTable[i + 1];
    }

    auto status = parseObject(raw, start, end);
    if (status != Status::OK) {
      return status;
    }
  }
  return Status::OK;
}

const IrProto& File::accessProtocol(int index) const
{
  return protocols.at(index);
}

uint16_t File::appendProtocol(const IrProto &proto)
{
  protocols.push_back(proto);
  return (protocols.size() - 1);
}

void File::removeProtocol(int index)
{
  if ((index < 0) || (index >= static_cast<int>(protocols.size()))) {
    return;
  }
  protocols.erase(protocols.begin() + index);
}

vector<uint8_t> File::serialise(uint32_t *crc) const
{
  int offset;
  vector<uint8_t> raw;
  vector<uint8_t> wrapper;
  vector<vector<uint8_t>> serialisedProtocols;

  raw.push_back(0); //fake first byte, fill later

  lib::setHarmony16_file(protocols.size(), raw); //append object count

  offset = raw.size() + 2 * protocols.size();
  for (const auto &prot : protocols) {
    lib::setHarmony16_file(offset, raw); //append object start offset

    auto protocol = prot.serialise(offset);
    offset = offset + protocol.size();

    serialisedProtocols.push_back(move(protocol));
  }

  //append objects
  for (const auto &s : serialisedProtocols) {
    raw.insert(raw.end(), s.begin(), s.end());
  }

  //add header
  raw.erase(raw.begin()); //remove fake byte
  raw.insert(raw.begin(), header.begin(), header.end());

  //add wrapper
  auto crc32 = lib::calcCrc32( { raw.begin(), raw.end() });
  lib::setHarmony32_file(crc32, wrapper);
  lib::setHarmony16_file(raw.size(), wrapper);
  lib::setHarmony16_file(0, wrapper);
  raw.insert(raw.begin(), wrapper.begin(), wrapper.end());

  if (crc != nullptr) {
    *crc = crc32;
  }

  return raw;
}

Status File::serialise(const string &filename, uint32_t *crc) const
{
  ofstream file(filename, ios::binary | ios::trunc); // binary + overwrite

  if (!file.is_open()) {
    return Status::ERROR_FILE;
  }

  auto raw = serialise(crc);
  file.write(reinterpret_cast<const char*>(raw.data()), raw.size());

  if (!file.good()) {
    return Status::ERROR_FILE;
  }
  return Status::OK;
}

Status File::serialiseIrStream(lib::TimingStream &out, uint16_t index,
    const IrProto::Data &data) const
{
  vector<Item> items;

  if (index >= protocols.size()) {
    return Status::ERROR_INDEX;
  }

  protocols[index].serialiseIrStream(items, data);
  items = compress(items);

  //we have list of coded bits (time + mark/pause).
  //we need timing stream (time-mark + time-pause).
  while (items.size() > 0) {
    const auto mark = items[0].first;
    const auto time_us = items[0].second;

    if (!mark) {
      //Block is always mark/pause, add empty mark
      out.addMarkPause( { 0, time_us });
      items.erase(items.begin());
      continue;
    }

    if (items.size() > 1) {
      const auto next_mark = items[1].first;
      const auto next_time_us = items[1].second;

      if (next_mark == true) {
        //Block is always mark/pause, add empty pause
        out.addMarkPause( { time_us, 0 });
        items.erase(items.begin());
      } else {
        //insert mark/pause
        out.addMarkPause( { time_us, next_time_us });
        items.erase(items.begin(), items.begin() + 2);
      }
    } else {
      //last item
      out.addMarkPause( { time_us, 0 });
      items.erase(items.begin());
    }
  }
  return Status::OK;
}

Status File::parseObject(const vector<uint8_t> &raw, uint16_t startOffset,
    uint16_t endOffset)
{
  vector<uint8_t> payload =
      { raw.begin() + startOffset, raw.begin() + endOffset };

  auto proto = IrProto(payload, startOffset);
  protocols.push_back(move(proto));

  return Status::OK;
}

}

