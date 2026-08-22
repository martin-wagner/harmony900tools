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
irProto::Code encodeSIRCS(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodeSIRCS20(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodeSamsung32(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodePhilipsRC5(uint32_t protocolIndex, std::string codeString, uint32_t address,uint32_t command, std::vector<bool> rawData);
irProto::Code encodePhilipsRC6(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);
irProto::Code encodePhilipsRC6A(uint32_t protocolIndex, std::string codeString, uint32_t address, uint32_t command, std::vector<bool> rawData);

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
      return encodeSIRCS(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::SIRCS15:
      return encodeSIRCS(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::SIRCS20:
      return encodeSIRCS20(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::Samsung32:
      return encodeSamsung32(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::PhilipsRC5:
      return encodePhilipsRC5(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::PhilipsRC6:
      return encodePhilipsRC6(protocolIndex, codeString, address, command, rawData);
    case document::data::CodeType::PhilipsRC6A:
      return encodePhilipsRC6A(protocolIndex, codeString, address, command, rawData);
    default:
      return irProto::Code();
  }
}

/** set and encode default values (address 0, command 0 is used as default) */
irProto::Code encodeDefaults(uint32_t protocolIndex, document::data::CodeType codeType,
    std::string &codeString, uint32_t &address, uint32_t &command,
    std::vector<bool> &rawData);

/** set address and command word width (in bits). returns false -- no data for given protocol*/
bool getCodeSize(document::data::CodeType codeType,
    const std::string &codeString, uint32_t &addressWidth, uint32_t &commandWidth);
inline bool getCodeSize(document::data::CodeType codeType,
    const QString &codeString, uint32_t &addressWidth, uint32_t &commandWidth)
{
  return getCodeSize(codeType, codeString.toStdString(), addressWidth, commandWidth);
}

/** get sub type list */
QStringList getSubTypes(document::data::CodeType codeType);

}
}
