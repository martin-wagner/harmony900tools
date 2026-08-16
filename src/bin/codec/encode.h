// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "decode.h"
#include "bin/irProto/code.h"

namespace binary
{
namespace codec
{

irProto::Code encodeNec(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodeKASEIKYO(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodeSIRCS12(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodeSamsung32(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodePhilipsRC5(uint32_t protocolIndex, std::string codeString, uint32_t address,uint32_t command, std::vector<bool> rawData);
irProto::Code encodePhilipsRC6(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);

/** encode data to "Code" frame */
inline irProto::Code encode(uint32_t protocolIndex, document::data::CodeType codeType,
    std::string codeString, uint32_t address, uint32_t command,
    std::vector<bool> rawData)
{
  switch (codeType) {
    case document::data::CodeType::NEC:
      return encodeNec(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::KASEIKYO:
      return encodeKASEIKYO(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::SIRCS12:
      return encodeSIRCS12(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::Samsung32:
      return encodeSamsung32(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::PhilipsRC5:
      return encodePhilipsRC5(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::PhilipsRC6:
      return encodePhilipsRC6(protocolIndex, codeString, address, command, rawData);
    default:
      return irProto::Code();
  }
}

}
}
