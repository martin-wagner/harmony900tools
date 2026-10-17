// SPDX-License-Identifier: LGPL-2.1-or-later

#include <cmath>

#include "code.h"

#include "lib/harmony_endian.h"
#include "cli/lib/lib.h"

using namespace std;

namespace binary
{
namespace irProto
{

vector<uint8_t> Code::bytesToList(uint8_t bitCount, uint8_t bits,
    const vector<uint8_t> &data)
{
  uint8_t value;

  vector<uint8_t> values;
  values.reserve(bitCount);

  for (uint16_t i = 0; i < bitCount; i++) {
    value = 0;

    for (uint8_t bit = 0; bit < bits; bit++) {
      auto bitIndex = i * bits + bit;
      auto byteIndex = bitIndex / 8;
      auto bitOffset = 7 - (bitIndex % 8);

      if (byteIndex < data.size()) {
        value = static_cast<uint8_t>((value << 1)
            | ((data[byteIndex] >> bitOffset) & 1));
      } else {
        value = static_cast<uint8_t>(value << 1);
      }
    }

    values.push_back(value);
  }

  return values;
}

vector<uint8_t> Code::u64bitsToList(uint8_t bitCount, uint64_t data)
{
  vector<uint8_t> values;
  values.reserve(bitCount);

  if (bitCount > 64) {
    return values;
  }

  for (int i = bitCount - 1; i >= 0; --i) {
    values.push_back((data >> i) & 1);
  }

  return values;
}

vector<uint8_t> Code::listToBytes(uint8_t bits, const vector<uint8_t> &values)
{
  vector<uint8_t> data((values.size() * bits + 7) / 8, 0);

  for (size_t i = 0; i < values.size(); i++) {
    for (uint8_t bit = 0; bit < bits; bit++) {
      auto bitIndex = i * bits + bit;
      auto byteIndex = bitIndex / 8;
      auto bitOffset = 7 - (bitIndex % 8);
      auto valueBit = (values[i] >> (bits - 1 - bit)) & 1;

      data[byteIndex] |= static_cast<uint8_t>(valueBit << bitOffset);
    }
  }

  return data;
}

uint64_t Code::listToU64bits(const vector<uint8_t> &values)
{
  uint64_t data = 0;

  for (uint8_t value : values) {
    data = (data << 1) | (value & 1);
  }

  return data;
}

//todo
//
//vector<bool> Code::bytesToBits(const vector<uint8_t> &data)
//{
//  vector<bool> bits;
//  bits.reserve(data.size() * 8);
//
//  for (auto byte : data) {
//    // iterate from MSB (bit 7) to LSB (bit 0)
//    for (int i = 7; i >= 0; --i) {
//      bits.push_back((byte >> i) & 1);
//    }
//  }
//
//  return bits;
//}
//
vector<bool> Code::u64toBits(uint8_t bitCount, uint64_t data)
{
  vector<bool> bits;
  bits.reserve(bitCount);

  for (int i = bitCount - 1; i >= 0; --i) {
    bits.push_back((data >> i) & 1);
  }

  return bits;
}
//
//vector<uint8_t> Code::bitsToBytes(const vector<bool> &bits)
//{
//  vector<uint8_t> data;
//  size_t numBytes = (bits.size() + 7) / 8; // ceil division
//  data.reserve(numBytes);
//
//  for (size_t i = 0; i < bits.size(); i += 8) {
//    uint8_t byte = 0;
//
//    for (int j = 0; j < 8; j++) {
//      byte = byte << 1;
//
//      if (i + j < bits.size()) {
//        byte |= bits[i + j] ? 1 : 0;
//      }
//      // else: implicit zero padding
//    }
//
//    data.push_back(byte);
//  }
//
//  return data;
//}
//
uint64_t Code::bitsToU64(const vector<bool> &bits)
{
  uint64_t data = 0;

  for (size_t i = 0; i < bits.size(); ++i) {
    data = (data << 1) | bits[i];
  }

  return data;
}

uint8_t Code::getBitsPerSymbol(uint8_t quantums)
{
  if (quantums <= 1) {
    return 0;
  }

  return static_cast<uint8_t>(ceil(log2(quantums)));
}

uint16_t Code::getDataBytes(uint16_t symbols, uint8_t bitsPerSymbol)
{
  if (symbols == 0 || bitsPerSymbol == 0) {
    return 0;
  }

  return (symbols * bitsPerSymbol + 7) / 8;
}

Status Code::parseGroup(vector<uint8_t> &data, vector<Group> &group)
{
  uint32_t i;

  try {
    auto recordCount = data.at(0);
    data.erase(data.begin(), data.begin() + 1);
    if (recordCount == 0) {
      //emtpy group
      return Status::OK;
    }

    for (i = 0; i < recordCount; ++i) {
      auto protoSection = data.at(0);
      if (protoSection >= protocol.getSectionCount()) {
        return Status::ERROR_PAYLOAD_FORMAT;
      }

      auto symbols = protocol.accessSection(protoSection).getBitCount();
      auto quantisation =
          protocol.accessSection(protoSection).getData().getQuantisation();
      auto bits = getBitsPerSymbol(quantisation);
      auto dataBytes = getDataBytes(symbols, bits);
      if (data.size() < (1 + dataBytes)) {
        return Status::ERROR_SIZE; //doesn't throw
      }
      auto groupData = bytesToList(symbols, bits,
          { data.begin() + 1, data.begin() + 1 + dataBytes });
      group.push_back(Group(protoSection, bits, groupData));

      data.erase(data.begin(), data.begin() + 1 + dataBytes);
    }
  } catch (out_of_range&) {
    return Status::ERROR_SIZE;
  }
  return Status::OK;
}

void Code::setGroup(const vector<pair<uint8_t, vector<uint8_t>>> &data,
    vector<Group> &group)
{
  group.clear();

  for (const auto& [protoSection, symbols] : data) {
    auto quantisation =
        protocol.accessSection(protoSection).getData().getQuantisation();
    auto bits = getBitsPerSymbol(quantisation);
    group.push_back(Group(protoSection, bits, symbols));
  }
}

void Code::writeGroup(const vector<Group> &group, vector<uint8_t> &data) const
{
  data.push_back(group.size());

  for (const auto &g : group) {
    data.push_back(g.getSection());
    auto bytes = listToBytes(g.getBits(), g.getData());
    for (auto &byte : bytes) {
      data.push_back(byte);
    }
  }
}

Code::Code() :
    Code(0xffff, IrProto())
{
}

Code::Code(uint16_t index, const IrProto &protocol) :
    protocol(protocol), index(index)
{
  reset();
}

Code::Code(uint16_t index, const IrProto &protocol, const vector<uint8_t> &code) :
    Code(index, protocol)
{
  parse(code);
}

Code::Code(uint16_t index, const IrProto &protocol, const string &code) :
    Code(index, protocol)
{
  parse(code);
}

void Code::reset()
{
  delay = DEFAULT_PRE_SEND_SILENCE;
  repeats = DEFAULT_REPEATS;
  start.clear();
  repeat.clear();
  finish.clear();
}

Status Code::parse(std::vector<uint8_t> code, int &index)
{
  index = -1;

  if (code.size() < MIN_SIZE) {
    return Status::ERROR_SIZE;
  }

  index = lib::parseHarmony16_file(code[0], code[1]);
  return Status::OK;
}

Status Code::parse(const std::string &code, int &index)
{
  auto data = lib::hexStringToBytes(code);
  return parse(data, index);
}

Status Code::parse(vector<uint8_t> code)
{
  reset();

  if (code.size() < MIN_SIZE) {
    return Status::ERROR_SIZE;
  }

  auto index = lib::parseHarmony16_file(code[0], code[1]);
  if (index != this->index) {
    return Status::ERROR_INDEX;
  }

  delay = lib::parseHarmony16_file(code[2], code[3]);
  repeats = code[4];

  code.erase(code.begin(), code.begin() + HEADER_SIZE);

  auto status = parseGroup(code, start);
  if (status != Status::OK) {
    reset();
    return status;
  }
  status = parseGroup(code, repeat);
  if (status != Status::OK) {
    reset();
    return status;
  }
  status = parseGroup(code, finish);
  if (status != Status::OK) {
    reset();
    return status;
  }
  //used up all bytes?
  if (!code.empty()) {
    reset();
    return Status::ERROR_SIZE;
  }
  return Status::OK;
}

Status Code::parse(const string &code)
{
  auto data = lib::hexStringToBytes(code);
  return parse(data);
}

Code Code::createSimpleRepeatEqData(uint16_t index, const IrProto &protocol,
    uint16_t delay, uint8_t bits, uint64_t data, uint8_t repeats)
{
  Code code(index, protocol);
  code.setDelay(delay);
  code.setRepeats(repeats);
  code.setRepeatGroups( { { 0, u64bitsToList(bits, data) } });
  return code;
}

Code Code::createSimpleRepeatEmpty(uint16_t index, const IrProto &protocol,
    uint16_t delay, uint8_t bits, uint64_t data, uint8_t repeats)
{
  Code code(index, protocol);
  code.setDelay(delay);
  code.setRepeats(repeats);
  code.setStartGroups( { { 0, u64bitsToList(bits, data) } });
  code.setRepeatGroups( { { 1, { } } });
  return code;
}

Code Code::createMultiRepeatEqData(uint16_t index, const IrProto &protocol,
    uint16_t delay, const std::vector<std::pair<uint8_t, uint64_t> > &data,
    uint8_t repeats)
{
  int i;
  std::vector<std::pair<uint8_t, std::vector<uint8_t>>> group;

  Code code(index, protocol);
  code.setDelay(delay);
  code.setRepeats(repeats);
  for (i = 0; i < data.size(); i++) {
    group.push_back( { i, u64bitsToList(data[i].first, data[i].second) });
  }
  code.setRepeatGroups(group);
  return code;
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
  code.push_back(repeats);
  writeGroup(start, code);
  writeGroup(repeat, code);
  writeGroup(finish, code);
  return code;
}

const IrProto::Data Code::getData() const
{
  uint8_t i;
  IrProto::Data data;

  for (const auto &g : start) {
    data.push_back( { g.getSection(), g.getData() });
  }
  for (i = 0; i < repeats; i++) {
    for (const auto &g : repeat) {
      data.push_back( { g.getSection(), g.getData() });
    }
  }
  for (const auto &g : finish) {
    data.push_back( { g.getSection(), g.getData() });
  }
  return data;
}

}
}
