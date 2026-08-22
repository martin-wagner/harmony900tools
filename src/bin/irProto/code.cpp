// SPDX-License-Identifier: LGPL-2.1-or-later

#include "code.h"

#include "lib/harmony_endian.h"
#include "cli/lib/lib.h"

using namespace std;

namespace binary
{
namespace irProto
{

vector<bool> Code::bytesToBits(const vector<uint8_t> &data)
{
  vector<bool> bits;
  bits.reserve(data.size() * 8);

  for (auto byte : data) {
    // iterate from MSB (bit 7) to LSB (bit 0)
    for (int i = 7; i >= 0; --i) {
      bits.push_back((byte >> i) & 1);
    }
  }

  return bits;
}

vector<bool> Code::u64tobits(uint8_t bitCount, uint64_t data)
{
  vector<bool> bits;
  bits.reserve(bitCount);

  for (int i = bitCount - 1; i >= 0; --i) {
    bits.push_back((data >> i) & 1);
  }

  return bits;
}

vector<uint8_t> Code::bitsToBytes(const vector<bool> &bits)
{
  vector<uint8_t> data;
  size_t numBytes = (bits.size() + 7) / 8; // ceil division
  data.reserve(numBytes);

  for (size_t i = 0; i < bits.size(); i += 8) {
    uint8_t byte = 0;

    for (int j = 0; j < 8; j++) {
      byte = byte << 1;

      if (i + j < bits.size()) {
        byte |= bits[i + j] ? 1 : 0;
      }
      // else: implicit zero padding
    }

    data.push_back(byte);
  }

  return data;
}

uint64_t Code::bitsToU64(const vector<bool> &bits)
{
  uint64_t data = 0;

  for (size_t i = 0; i < bits.size(); ++i) {
    data = (data << 1) | bits[i];
  }

  return data;
}

Status Code::parseFlat(const vector<uint8_t> &data)
{
  //min 1 data byte + end code
  if (data.size() < 3) {
    return Status::ERROR_SIZE;
  }

  //ends with 0x01 0x01. use of this is unknown
  if ((data[data.size() - 1] != 1) || (data[data.size() - 2] != 1)) {
    cout << "Error: xml <code> flat/end != 0x01" << endl;
    return Status::ERROR_PAYLOAD_FORMAT;
  }
  auto bits = bytesToBits( { data.begin(), data.end() - 2 });
  sections.push_back(Section(0, bits));

  return Status::OK;
}

Status Code::parseSingleSection(const vector<uint8_t> &data)
{
  if (data.size() < 1) {
    return Status::ERROR_SIZE;
  }

  auto bits = bytesToBits( { data.begin(), data.end() });
  sections.push_back(Section(0, bits));

  return Status::OK;
}

Status Code::parseMultiSection(uint8_t expectedSectionCount,
    vector<uint8_t> data)
{
  int i;

  if (data.size() < (2 * expectedSectionCount)) {
    return Status::ERROR_SIZE;
  }

  for (i = 0; i < expectedSectionCount; i++) {
    auto bits = bytesToBits( { data.begin(), data.begin() + 2 });
    sections.push_back(Section(i, bits));
    data.erase(data.begin(), data.begin() + 2);
  }
  return Status::OK;
}

void Code::writeSections(std::vector<uint8_t> &data) const
{
  for (const auto &s : sections) {
    auto bytes = bitsToBytes(s.getData());
    for (auto &byte : bytes) {
      data.push_back(byte);
    }
  }
}

Code::Code()
{
}

Code::Code(const vector<uint8_t> &code)
{
  parse(code);
}

Code::Code(const string &code)
{
  parse(code);
}

Status Code::parse(vector<uint8_t> code)
{
  sections.clear();

  if (code.size() < (HEADER_SIZE + FOOTER_SIZE)) {
    return Status::ERROR_SIZE;
  }
  if (code[code.size() - 1] != 0) {
    return Status::ERROR_FILE_FORMAT;
  }
  code.erase(code.end() - 1);

  index = lib::parseHarmony16_file(code[0], code[1]);
  delay = lib::parseHarmony16_file(code[2], code[3]);
  dataFrameTxCount = code[4];
  haveRepeatFrame = code[5] & 0x01;
  if (haveRepeatFrame && (dataFrameTxCount > 1)) {
    //fixme seems to exist. no idea how this is supposed to work then.
    //use either one...
    cout << "Error: xml <code> multi-tx + repeat selected" << endl;
    return Status::ERROR_PAYLOAD_FORMAT;
  }

  ctrl = static_cast<Ctrl>(code[6]);
  switch (ctrl) {
    case Ctrl::FLAT:
      return parseFlat( { code.begin() + 7, code.end() });
    case Ctrl::SECTIONS_1:
      if (code[7] != 0) {
        cout << "Error: xml <code> ss pad7 != 0 (" << code[7] << ")" << endl;
        return Status::ERROR_PAYLOAD_FORMAT;
      }
      return parseSingleSection( { code.begin() + 8, code.end() });
    default:
      if (code[7] != 0) {
        cout << "Error: xml <code> ms pad7 != 0 (" << code[7] << ")" << endl;
        return Status::ERROR_PAYLOAD_FORMAT;
      }
      return parseMultiSection(static_cast<uint8_t>(ctrl), {
        code.begin() + 8,
        code.end() });
  }
}

void Code::createFlat(uint8_t index, int delay, uint8_t bits, uint64_t data)
{
  sections.clear();

  this->index = index;
  this->delay = delay;
  dataFrameTxCount = 1;
  haveRepeatFrame = 0;
  ctrl = Ctrl::FLAT;
  auto section = Section(0, u64tobits(bits, data));
  sections.push_back(section);
}

void Code::createSingleSection(uint8_t index, int delay, uint8_t bits,
    uint64_t data)
{
  sections.clear();

  this->index = index;
  this->delay = delay;
  dataFrameTxCount = 1;
  haveRepeatFrame = 0;
  ctrl = Ctrl::SECTIONS_1;
  auto section = Section(0, u64tobits(bits, data));
  sections.push_back(section);
}

void Code::createMultiSection(uint8_t index, int delay,
    const std::vector<std::pair<uint8_t, uint16_t> > &data)
{
  int i;

  sections.clear();

  this->index = index;
  this->delay = delay;
  dataFrameTxCount = 1;
  haveRepeatFrame = 0;
  ctrl = static_cast<Ctrl>(data.size());
  for (i = 0; i < data.size(); i++) {
    auto section = Section(i, u64tobits(data[i].first, data[i].second));
    sections.push_back(section);
  }
}

Status Code::parse(const string &code)
{
  auto data = lib::hexStringToBytes(code);
  return parse(data);
}

string Code::serialiseStr() const
{
  auto data = serialiseVec();
  return lib::bytesToHexString(data);
}

vector<uint8_t> Code::serialiseVec() const
{
  vector<uint8_t> code;

  lib::setHarmony16_file(index, code);
  lib::setHarmony16_file(delay, code);
  code.push_back(dataFrameTxCount);
  code.push_back(haveRepeatFrame);
  code.push_back(static_cast<uint8_t>(ctrl));
  switch (ctrl) {
    case Ctrl::FLAT:
      writeSections(code);
      code.push_back(0x01); //footer
      code.push_back(0x01);
      break;
    case Ctrl::SECTIONS_1:
      code.push_back(0); //padding byte;
      writeSections(code);
      break;
    default:
      code.push_back(0); //padding byte;
      writeSections(code);
      break;
  }
  code.push_back(0); //terminator
  return code;
}

const IrProto::Data Code::getData() const
{
  uint8_t i;
  IrProto::Data data;

  for (i = 0; i < dataFrameTxCount; i++) {
    for (const auto &s : sections) {
      data.push_back( { s.getIndex(), s.getData() });
    }
  }
  if (haveRepeatFrame) {
    data.push_back( { static_cast<int>(sections.size()), { } });
  }
  return data;
}

}
}
