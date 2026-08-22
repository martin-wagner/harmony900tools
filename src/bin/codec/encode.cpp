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
      data = irProto::Code::bitsToU64(rawData);
      break;
  }
  code.createFlat(protocolIndex, irProto::Code::DEFAULT_DELAY, rawData.size(),
      data);
  code.setRepeatFrame(1);
  return code;
}

irProto::Code encodeKASEIKYO(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data = irProto::Code::bitsToU64(rawData);
  irProto::Code code;
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
  code.createSingleSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      rawData.size(), data);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodeSIRCS(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  irProto::Code code;
  IRsend send(0, false, false);

  data = send.encodeSony(rawData.size(), command, address);
  code.createSingleSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      rawData.size(), data);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodeSIRCS20(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  irProto::Code code;
  IRsend send(0, false, false);

  data = send.encodeSony(rawData.size(), command & 0x7f, address,
      (command >> 7) & 0xff);
  code.createSingleSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      rawData.size(), data);
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

  data = send.encodeSAMSUNG(address, command);
  code.createSingleSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      rawData.size(), data);
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
  code.createSingleSection(protocolIndex, irProto::Code::DEFAULT_DELAY, 13,
      data);
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

  address = address | 0x100000; //always 1 bit
  data = send.encodeRC6(address, command, kRC6Mode0Bits);

  sections.push_back( { 16, 0x8001 });  //start
  sections.push_back( { 16, 0x0002 });  //toggle bit
  sections.push_back( { 16, data }); //16 data bits

  code.createMultiSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      sections);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodePhilipsRC6A(uint32_t protocolIndex, string codeString,
    uint32_t address, uint32_t command, vector<bool> rawData)
{
  uint64_t data;
  irProto::Code code;
  IRsend send(0, false, false);

  data = send.encodeRC6(address, command, kRC6_36Bits);

  code.createSingleSection(protocolIndex, irProto::Code::DEFAULT_DELAY,
      rawData.size(), data);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(irProto::Code::DEFAULT_REPEATS);
  return code;
}

irProto::Code encodeDefaults(uint32_t protocolIndex,
    document::data::CodeType codeType, string &codeString, uint32_t &address,
    uint32_t &command, vector<bool> &rawData)
{
  switch (codeType) {
    case document::data::CodeType::NEC: {
      rawData = irProto::Code::u64tobits(32, 0x00ff00ff); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::NEC);
      return encodeNec(protocolIndex, codeString, address, command, rawData);
    }
    case document::data::CodeType::KASEIKYO: {
      rawData = irProto::Code::u64tobits(48, uint64_t(0x000000000000)); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::PANASONIC);
      return encodeKASEIKYO(protocolIndex, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::SIRCS12: {
      rawData = irProto::Code::u64tobits(12, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SONY);
      return encodeSIRCS(protocolIndex, codeString, address, command, rawData);
    }
    case document::data::CodeType::SIRCS15: {
      rawData = irProto::Code::u64tobits(15, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SONY);
      return encodeSIRCS(protocolIndex, codeString, address, command, rawData);
    }
    case document::data::CodeType::SIRCS20: {
      rawData = irProto::Code::u64tobits(20, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SONY);
      return encodeSIRCS(protocolIndex, codeString, address, command, rawData);
    }
    case document::data::CodeType::Samsung32: {
      rawData = irProto::Code::u64tobits(32, 0x00000000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::SAMSUNG);
      return encodeSamsung32(protocolIndex, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::PhilipsRC5: {
      rawData = irProto::Code::u64tobits(12, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::RC5);
      return encodePhilipsRC5(protocolIndex, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::PhilipsRC6: {
      rawData = irProto::Code::u64tobits(16, 0x000); //addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::RC6);
      return encodePhilipsRC6(protocolIndex, codeString, address, command,
          rawData);
    }
    case document::data::CodeType::PhilipsRC6A: {
      rawData = irProto::Code::u64tobits(31, 0x00000000); //mnf 0, addr 0, cmd 0
      address = 0;
      command = 0;
      codeString = typeToString(decode_type_t::RC6);
      return encodePhilipsRC6A(protocolIndex, codeString, address, command,
          rawData);
    }
    default: {
      rawData.clear();
      address = 0;
      command = 0;
      codeString = "";
      return irProto::Code();
    }
  }
}

bool getCodeSize(document::data::CodeType codeType, const string &codeString,
    uint32_t &addressWidth, uint32_t &commandWidth)
{
  switch (codeType) {
    case document::data::CodeType::NEC:
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
      ret.push_back(QString::fromStdString(typeToString(decode_type_t::NEC)));
      ret.push_back(
          QString::fromStdString(typeToString(decode_type_t::NEC_LIKE)));
      break;
    case document::data::CodeType::KASEIKYO:
      ret.push_back(
          QString::fromStdString(typeToString(decode_type_t::PANASONIC)));
      ret.push_back(QString::fromStdString(typeToString(decode_type_t::DENON)));
      break;
    case document::data::CodeType::SIRCS12:
    case document::data::CodeType::SIRCS15:
    case document::data::CodeType::SIRCS20:
      ret.push_back(QString::fromStdString(typeToString(decode_type_t::SONY)));
      break;
    case document::data::CodeType::Samsung32:
      ret.push_back(
          QString::fromStdString(typeToString(decode_type_t::SAMSUNG)));
      break;
    case document::data::CodeType::PhilipsRC5:
      ret.push_back(QString::fromStdString(typeToString(decode_type_t::RC5)));
      ret.push_back(QString::fromStdString(typeToString(decode_type_t::RC5X)));
      break;
    case document::data::CodeType::PhilipsRC6:
    case document::data::CodeType::PhilipsRC6A:
      ret.push_back(QString::fromStdString(typeToString(decode_type_t::RC6)));
      break;
    default:
      break;
  }
  return ret;
}

}
}
