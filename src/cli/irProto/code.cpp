/*
 * code.cpp
 *
 *  Created on: Apr 4, 2026
 *      Author: martin
 */

#include "code.h"
#include "cli/lib/lib.h"
#include "cli/lib/binary.h"

using namespace std;

namespace irProto
{

std::vector<bool> Code::bytesToBits(const std::vector<uint8_t> &data)
{
  std::vector<bool> bits;
  bits.reserve(data.size() * 8);

  for (auto byte : data) {
    // iterate from MSB (bit 7) to LSB (bit 0)
    for (int i = 7; i >= 0; --i) {
      bits.push_back((byte >> i) & 1);
    }
  }

  return bits;
}

std::vector<uint8_t> Code::bitsToBytes(const std::vector<bool> &bits)
{
  std::vector<uint8_t> data;
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

Status Code::parseFlat(const std::vector<uint8_t> &data)
{
  //min 1 data byte + end code
  if (data.size() < 3) {
    return Status::ERROR_SIZE;
  }

  //ends with 0x01 0x01. use of this is unknown
  if ((data[data.size() - 1] != 1) || (data[data.size() - 2] != 1)) {
    cout << "Warning: xml <code> flat/end != 0x01" << endl;
    //ignore
  }
  auto bits = bytesToBits( { data.begin(), data.end() - 2 });
  for (int i = 0; i < dataFrameTxCount; i++) {
    sections.push_back(Section(0, bits));
  }

  if (haveRepeatFrame) {
    sections.push_back(Section(1, { }));
  }

  return Status::OK;
}

Status Code::parseSingleSection(const std::vector<uint8_t> &data)
{
  if (data.size() < 1) {
    return Status::ERROR_SIZE;
  }

  auto bits = bytesToBits( { data.begin(), data.end() });
  for (int i = 0; i < dataFrameTxCount; i++) {
    sections.push_back(Section(0, bits));
  }

  if (haveRepeatFrame) {
    sections.push_back(Section(1, { }));
  }

  return Status::OK;
}

Status Code::parseMultiSection(const std::vector<uint8_t> &data)
{
  int i;
  int j;
  auto tmp = data;

  if (tmp.size() < (2 * dataSectionCount)) {
    return Status::ERROR_SIZE;
  }

  for (i = 0; i < dataFrameTxCount; i++) {
    for (j = 0; j < dataSectionCount; j++) {
      auto bits = bytesToBits( { tmp.begin(), tmp.begin() + 1 });
      sections.push_back(Section(j, bits));
      tmp.erase(tmp.begin(), tmp.begin() + 1);
    }
  }

  if (haveRepeatFrame) {
    //no idea where the repeat needs to be placed. just assume it is at the end...
    sections.push_back(Section(j + 1, { }));
  }

  return Status::OK;
}

Code::Code()
{
}

Code::Code(const string &code)
{
  parse(code);
}

Status Code::parse(const string &code)
{
  int dataSectionCount;
  int dataSectionBegin;

  sections.clear();

  auto data = lib::hexStringToBytes(code);
  if (data.size() < (HEADER_SIZE + FOOTER_SIZE)) {
    return Status::ERROR_SIZE;
  }
  if (data[data.size() - 1] != 0) {
    return Status::ERROR_FILE_FORMAT;
  }
  data.erase(data.end());

  index = lib::parseHarmony16_file(data[0], data[1]);
  ticks = lib::parseHarmony16_file(data[2], data[3]);
  dataFrameTxCount = data[4];
  haveRepeatFrame = data[5];
  if (haveRepeatFrame && (dataFrameTxCount > 1)) {
    //use either one...
    cout << "Warning: xml <code> multi-tx + repeat selected" << endl;
    //ignore
  }

  ctrl = static_cast<Ctrl>(data[6]);
  switch (ctrl) {
    case Ctrl::FLAT:
      dataSectionCount = 1;
      return parseFlat( { data.begin() + 7, data.end() });
    case Ctrl::SECTIONS_1:
      if (data[7] != 0) {
        cout << "Warning: xml <code> ss pad7 != 0 (" << data[7] << ")" << endl;
        //ignore
      }
      dataSectionCount = 1;
      return parseSingleSection( { data.begin() + 8, data.end() });
    default:
      if (data[7] != 0) {
        cout << "Warning: xml <code> ms pad7 != 0 (" << data[7] << ")" << endl;
        //ignore
      }
      dataSectionCount = static_cast<uint8_t>(ctrl);
      return parseMultiSection( { data.begin() + 8, data.end() });
  }
}

const IrProto::Data Code::getData() const
{
  IrProto::Data data;

  for (const auto& s : sections) {
    data.push_back( { s.getIndex(), s.getData()});
  }
  return data;
}

}

