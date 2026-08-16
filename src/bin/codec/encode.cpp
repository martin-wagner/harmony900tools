// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef UNIT_TEST
#define IRREMOTEESP8266_DEFINED_UNIT_TEST
#define UNIT_TEST 1 //make IRemoteESP8266 run without the ESP8266...
#endif
#include <IRsend.h>
#include <IRutils.h>
#ifdef IRREMOTEESP8266_DEFINED_UNIT_TEST
#undef UNIT_TEST
#undef IRREMOTEESP8266_DEFINED_UNIT_TEST
#endif

#include "lib/bits.h"
#include "encode.h"

using namespace std;

namespace binary
{
namespace codec
{

//functions must match "decode.cpp -- parse" and "ir_protocols.json" coding params!

irProto::Code encodeNec(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  irProto::Code code;
  IRsend send(0, false, false);

  switch (strToDecodeType(codeString.c_str())) {
    case decode_type_t::NEC:
      data = send.encodeNEC(address, command);
      break;
    case decode_type_t::NEC_LIKE:
      data = (address & 0xffff) << 16 | (command & 0xffff);
      break;
    default:
      //should not happen
      data = lib::bitsTou64Msb(rawData);
      break;
  }
  code.createSingleSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      rawData.size(), data);
  code.setRepeatFrame(1);
  return code;
}

irProto::Code encodeKASEIKYO(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data = lib::bitsTou64Msb(rawData);
  irProto::Code code;
  IRsend send(0, false, false);

  switch (strToDecodeType(codeString.c_str())) {
    case decode_type_t::PANASONIC:
    case decode_type_t::DENON:
      data = send.encodePanasonic(address, (data >> 16) & 0xff,
          (data >> 8) & 0xff, data & 0xff);
      break;
    default:
      //should not happen, keep as-is
      break;
  }
  code.createFlat(protocolIndex, irProto::Code::DEFAULT_DELAY, rawData.size(),
      data);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodeSIRCS12(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  irProto::Code code;
  IRsend send(0, false, false);

  data = send.encodeSony(rawData.size(), command, address);
  code.createFlat(protocolIndex, irProto::Code::DEFAULT_DELAY, rawData.size(),
      data);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodeSamsung32(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  irProto::Code code;
  IRsend send(0, false, false);

  data = send.encodeSAMSUNG(command, address);
  code.createFlat(protocolIndex, irProto::Code::DEFAULT_DELAY, rawData.size(),
      data);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodePhilipsRC5(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  irProto::Code code;
  IRsend send(0, false, false);

  switch (rawData.size()) {
    case 13:
      data = send.encodeRC5X(address, command);
      break;
    case 12:
    default:
      data = send.encodeRC5(address, command);
      data = data | 0b1000000000000; //manually set second start bit
      break;
  }
  code.createFlat(protocolIndex, irProto::Code::DEFAULT_DELAY, 13, data);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodePhilipsRC6(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  vector<pair<uint8_t, uint16_t>> sections;
  irProto::Code code;
  IRsend send(0, false, false);

  data = send.encodeRC6(command, address, kRC6Mode0Bits);

  sections.push_back( { 4, 0x08 });  //start bit
  sections.push_back( { 1, 0x00 });  //toggle bit
  sections.push_back( { 16, data }); //16 data bits

  code.createMultiSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      sections);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

}
}

