// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/qtHelpers.h"

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

#include "encode.h"

using namespace std;
using namespace binary::irProto;

namespace binary
{
namespace codec
{

//functions must match "decode.cpp -- parse" and "ir_protocols.json" coding params!

Code encodeNec(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data;
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
      data = Code::listToU64bits(rawData);
      break;
  }
  return Code::createSimpleRepeatEmpty(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, rawData.size(), data);
}

Code encodeKASEIKYO(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data = Code::listToU64bits(rawData);
  IRsend send(0, false, false);

  switch (strToDecodeType(codeString.c_str())) {
    case decode_type_t::PANASONIC:
    case decode_type_t::DENON:
      data = send.encodePanasonic(address, (data >> 24) & 0xff,
          (data >> 16) & 0xff, (data >> 8) & 0xff);
      break;
    default:
      //should not happen, keep as-is
      break;
  }
  return Code::createSimpleRepeatEqData(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, rawData.size(), data);
}

Code encodeSIRCS(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data;
  IRsend send(0, false, false);

  data = send.encodeSony(rawData.size(), command, address);
  return Code::createSimpleRepeatEqData(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, rawData.size(), data);
}

Code encodeSIRCS20(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data;
  IRsend send(0, false, false);

  data = send.encodeSony(rawData.size(), command & 0x7f, address,
      (command >> 7) & 0xff);
  return Code::createSimpleRepeatEqData(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, rawData.size(), data);
}

Code encodeSamsung32(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data;
  IRsend send(0, false, false);

  data = send.encodeSAMSUNG(address, command);
  return Code::createSimpleRepeatEqData(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, rawData.size(), data);
}

Code encodePhilipsRC5(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data;
  IRsend send(0, false, false);

  switch (strToDecodeType(codeString.c_str())) {
    case decode_type_t::RC5:
    default:
      data = send.encodeRC5(address, command);
      data = data | 0b1000000000000; //manually set second start bit
      break;
    case decode_type_t::RC5X:
      data = send.encodeRC5X(address, command);
      break;
  }
  return Code::createSimpleRepeatEqData(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, rawData.size(), data);
}

Code encodePhilipsRC6(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data;
  vector<pair<uint8_t, uint64_t>> sections;
  IRsend send(0, false, false);

  address = address | 0x100000; //always 1 bit
  data = send.encodeRC6(address, command, kRC6Mode0Bits);

  sections.push_back( { 4, 0x08 });  //start
  sections.push_back( { 1, 0x00 });  //toggle bit
  sections.push_back( { 16, data }); //16 data bits
  return Code::createMultiRepeatEqData(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, sections);
}

Code encodePhilipsRC6A(uint32_t protocolIndex, const IrProto &protocol,
    string codeString, uint32_t address, uint32_t command,
    vector<uint8_t> rawData)
{
  uint64_t data;
  IRsend send(0, false, false);

  data = send.encodeRC6(address, command, kRC6_36Bits);
  return Code::createSimpleRepeatEqData(protocolIndex, protocol,
      Code::DEFAULT_PRE_SEND_SILENCE, rawData.size(), data);
}

Code encodeDefaults(uint32_t protocolIndex, const IrProto &protocol,
    document::data::CodeType codeType, string &codeString, uint32_t &address,
    uint32_t &command, vector<uint8_t> &rawData)
{
  switch (codeType) {
    case document::data::CodeType::NEC:
    case document::data::CodeType::NEC40kHz: {
      rawData = Code::u64bitsToList(32, 0x00ff00ff); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::NEC);
      return encodeNec(protocolIndex, protocol, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::KASEIKYO:
    case document::data::CodeType::KASEIKYO36kHz: {
      rawData = Code::u64bitsToList(48, uint64_t(0x000000000000)); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::PANASONIC);
      return encodeKASEIKYO(protocolIndex, protocol, codeString, address,
          command, rawData);
    }
    case document::data::CodeType::SIRCS12: {
      rawData = Code::u64bitsToList(12, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SONY);
      return encodeSIRCS(protocolIndex, protocol, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::SIRCS15: {
      rawData = Code::u64bitsToList(15, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SONY);
      return encodeSIRCS(protocolIndex, protocol, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::SIRCS20: {
      rawData = Code::u64bitsToList(20, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SONY);
      return encodeSIRCS(protocolIndex, protocol, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::Samsung32: {
      rawData = Code::u64bitsToList(32, 0x00000000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SAMSUNG);
      return encodeSamsung32(protocolIndex, protocol, codeString, address,
          command, rawData);
    }
    case document::data::CodeType::PhilipsRC5: {
      rawData = Code::u64bitsToList(12, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::RC5);
      return encodePhilipsRC5(protocolIndex, protocol, codeString, address,
          command, rawData);
    }
    case document::data::CodeType::PhilipsRC6: {
      rawData = Code::u64bitsToList(16, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::RC6);
      return encodePhilipsRC6(protocolIndex, protocol, codeString, address,
          command, rawData);
    }
    case document::data::CodeType::PhilipsRC6A: {
      rawData = Code::u64bitsToList(31, 0x00000000); //mnf 0, addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::RC6);
      return encodePhilipsRC6A(protocolIndex, protocol, codeString, address,
          command, rawData);
    }
    default: {
      rawData.clear();
      address = 0;
      command = 0;
      codeString = "";
      return Code(protocolIndex, protocol);
    }
  }
}

bool getCodeSize(document::data::CodeType codeType, const string &codeString,
    uint32_t &addressWidth, uint32_t &commandWidth)
{
  switch (codeType) {
    case document::data::CodeType::NEC:
    case document::data::CodeType::NEC40kHz:
      switch (strToDecodeType(codeString.c_str())) {
        case decode_type_t::NEC:
          addressWidth = 16;
          commandWidth = 8;
          return true;
        case decode_type_t::NEC_LIKE:
          addressWidth = 16;
          commandWidth = 16;
          return true;
        default:
          break;
      }
      break;
    case document::data::CodeType::KASEIKYO:
    case document::data::CodeType::KASEIKYO36kHz:
      //special coding, more than address / command
      break;
    case document::data::CodeType::SIRCS12:
      addressWidth = 5;
      commandWidth = 7;
      return true;
    case document::data::CodeType::SIRCS15:
      addressWidth = 8;
      commandWidth = 7;
      return true;
    case document::data::CodeType::SIRCS20:
      addressWidth = 5;
      commandWidth = 7;
      return true;
    case document::data::CodeType::Samsung32:
      addressWidth = 16;
      commandWidth = 16;
      return true;
    case document::data::CodeType::PhilipsRC5:
      switch (strToDecodeType(codeString.c_str())) {
        case decode_type_t::RC5:
          addressWidth = 5;
          commandWidth = 6;
          return true;
        case decode_type_t::RC5X:
          addressWidth = 5;
          commandWidth = 7;
          return true;
        default:
          break;
      }
      break;
    case document::data::CodeType::PhilipsRC6:
      addressWidth = 8;
      commandWidth = 8;
      return true;
    case document::data::CodeType::PhilipsRC6A:
      addressWidth = 23;
      commandWidth = 8;
      return true;
    default:
      break;
  }
  addressWidth = 0;
  commandWidth = 0;
  return false;
}

QStringList getSubTypes(document::data::CodeType codeType)
{
  QStringList ret;

  switch (codeType) {
    case document::data::CodeType::NEC:
    case document::data::CodeType::NEC40kHz:
      ret.push_back(qstr(typeToString(decode_type_t::NEC)));
      ret.push_back(qstr(typeToString(decode_type_t::NEC_LIKE)));
      break;
    case document::data::CodeType::KASEIKYO:
    case document::data::CodeType::KASEIKYO36kHz:
      ret.push_back(qstr(typeToString(decode_type_t::PANASONIC)));
      ret.push_back(qstr(typeToString(decode_type_t::DENON)));
      break;
    case document::data::CodeType::SIRCS12:
    case document::data::CodeType::SIRCS15:
    case document::data::CodeType::SIRCS20:
      ret.push_back(qstr(typeToString(decode_type_t::SONY)));
      break;
    case document::data::CodeType::Samsung32:
      ret.push_back(qstr(typeToString(decode_type_t::SAMSUNG)));
      break;
    case document::data::CodeType::PhilipsRC5:
      ret.push_back(qstr(typeToString(decode_type_t::RC5)));
      ret.push_back(qstr(typeToString(decode_type_t::RC5X)));
      break;
    case document::data::CodeType::PhilipsRC6:
    case document::data::CodeType::PhilipsRC6A:
      ret.push_back(qstr(typeToString(decode_type_t::RC6)));
      break;
    default:
      break;
  }
  return ret;
}

}
}
