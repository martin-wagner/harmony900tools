/*
 * file.cpp
 *
 *  Created on: Apr 4, 2026
 *      Author: martin
 */

#include <fstream>

#include "file.h"
#include "cli/lib/binary.h"
#include "cli/lib/crc32.h"

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

void TimingSectionIrHeader::serialiseIrStream(vector<Item> out) const
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
    vector<Item> out) const
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
  mask = lib::parseHarmony16_file(data[2], data[3]);
  interval = bitCount = lib::parseHarmony32_file(data[4], data[5], data[6],
      data[7]);
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

  if ((pSoF >= HEADER_SIZE) && (pSoF < data.size())) {
    auto size = data[pSoF] * 2 + 1;
    sof = TimingSectionIrHeader(
        { data.begin() + pSoF, data.begin() + pSoF + size });
  }
  if ((pEoF >= HEADER_SIZE) && (pEoF < data.size())) {
    auto size = data[pEoF] * 2 + 1;
    eof = TimingSectionIrHeader(
        { data.begin() + pEoF, data.begin() + pEoF + size });
  }
  if ((pData >= HEADER_SIZE) && (pData < data.size())) {
    auto size = static_cast<int>(ctrl1) * 4;
    this->data = TimingSectionIrPayload(
        { data.begin() + pSoF, data.begin() + pSoF + size });
  }
}

void TimingSection::setBitCount(uint16_t count)
{
  bitCount = count;
}

void TimingSection::setMask(uint16_t mask)
{
  this->mask = mask;
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
  lib::setHarmony16_file(mask, data);
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

void TimingSection::serialiseIrStream(std::vector<Item> out,
    const vector<bool> &data) const
{
  //don't do convertion of single timings to actual stream here!
  //protocol can append multiple timing sections into one frame.
  sof.serialiseIrStream(out);
  for (auto bit : data) {
    this->data.serialiseIrStream(bit, out);
  }
  eof.serialiseIrStream(out);


  //  auto stream = this->stream; //copy
  //
  //  while (stream.size() > 0) {
  //    const auto &current = stream[0];
  //
  //    if (stream.size() > 1) {
  //      const auto &next = stream[1];
  //
  //      if ((current.first == true) && (next.first == false)) {
  //        //mark/pause, insert
  //        out.addMarkPause( {current.second, next.second });
  //        stream.erase(stream.begin(), stream.begin() + 2);
  //      } else if ((current.first == true) && (next.first == true)) {
  //        //mark/mark, add empty pause, insert
  //        out.addMarkPause( {current.second, 0 });
  //        stream.erase(stream.begin(), stream.begin() + 1);
  //      } else {
  //        //pause, add empty mark, insert
  //        out.addMarkPause( {0, current.second });
  //        stream.erase(stream.begin(), stream.begin() + 1);
  //      }
  //    } else {
  //      if (current.first == true) {
  //        //mark, add empty pause, insert
  //        out.addMarkPause( {current.second, 0 });
  //      } else {
  //        //pause, add empty mark, insert
  //        out.addMarkPause( {0, current.second });
  //      }
  //      stream.erase(stream.begin(), stream.begin() + 1);
  //    }
  //  } todo

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
      { data.begin() + HEADER_SIZE - 1, data.begin() + HEADER_SIZE - 1
          + 2 * timingSectionCount }, offsetTable);

  //offsets need to be within data
  for (auto &o : offsetTable) {
    o = o - offset;
    if ((o) > data.size()) {
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

    serialisedSections.push_back(move(section));
  }

  //append sections
  for (const auto &s : serialisedSections) {
    data.insert(data.end(), s.begin(), s.end());
  }

  return data;
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
    cout << "Warning: IrProto.bin CRC mismatch, file: " << hex << expectedCrc32
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

