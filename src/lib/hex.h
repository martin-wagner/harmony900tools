// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#include "lib/timestamp.h"

namespace lib
{

//string hh:mm:ss:MMM text 00 00 00 ...
inline std::string writeHex(const std::string &text, const std::vector<uint8_t> &data)
{
  std::stringstream str;

  str << writeTime() << text;

  for (auto b : data) {
    str << std::hex << std::setw(2) << std::setfill('0') << (int) b << " ";
  }
  str << std::dec << std::endl;

  return str.str();
}

//string hh:mm:ss:MMM text 0000 0000 0000 ...
inline std::string writeHex(const std::string &text, const std::vector<uint16_t> &data)
{
  std::stringstream str;

  str << writeTime() << text;

  for (auto w : data) {
    str << std::hex << std::setw(4) << std::setfill('0') << (int) w << " ";
  }
  str << std::dec << std::endl;

  return str.str();
}

//string hh:mm:ss:MMM text 00000 00000 00000 ...
inline std::string writeData(const std::string &text, const std::vector<uint16_t> &data)
{
  std::stringstream str;

  str << writeTime() << text;

  for (auto w : data) {
    str << std::setw(5) << std::setfill('0') << (int) w << " ";
  }
  str << std::endl;

  return str.str();
}

//convert hex string 0x123456789abcdef to vector<uint8_t> [12, 34, ...]
inline std::vector<uint8_t> hexStringToBytes(const std::string &code)
{
  std::vector<uint8_t> data;
  std::string hex = code;

  // remove optional "0x" prefix
  if (hex.rfind("0x", 0) == 0 || hex.rfind("0X", 0) == 0) {
    hex = hex.substr(2);
  }

  if (hex.size() % 2 != 0) {
    return data;
  }

  for (int i = 0; i < hex.size(); i += 2) {
    auto byteStr = hex.substr(i, 2);
    auto byte = static_cast<uint8_t>(stoul(byteStr, nullptr, 16));
    data.push_back(byte);
  }

  return data;
}

//convert hex string 0x123456789abcdef to vector<bool> [true, false, ...]
inline std::vector<bool> hexStringToBits(const std::string &code, size_t bitCount)
{
  std::vector<uint8_t> bytes = hexStringToBytes(code);
  std::vector<bool> bits;
  bits.reserve(bitCount);

  for (size_t i = 0; i < bitCount && (i / 8) < bytes.size(); i++) {
    bits.push_back((bytes[i / 8] & static_cast<uint8_t>(1u << (i % 8))) != 0);
  }

  return bits;
}

// convert vector<uint8_t> to hex string "0x1234abcd..."
inline std::string bytesToHexString(const std::vector<uint8_t> &data, bool addPrefix = true)
{
  static const char *hexChars = "0123456789ABCDEF";

  std::string out;
  out.reserve(data.size() * 2 + (addPrefix ? 2 : 0));

  if (addPrefix) {
    out += "0x";
  }

  for (size_t i = 0; i < data.size(); i++) {
    uint8_t byte = data[i];
    out.push_back(hexChars[(byte >> 4) & 0x0F]);
    out.push_back(hexChars[byte & 0x0F]);
  }

  return out;
}

// convert vector<bool> to hex string "0x1234abcd..."
inline std::string bitsToHexString(const std::vector<bool> &data, bool addPrefix = true)
{
  std::vector<uint8_t> bytes((data.size() + 7) / 8, 0);

  for (size_t i = 0; i < data.size(); i++) {
    if (data[i] == true) {
      bytes[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
    }
  }

  return bytesToHexString(bytes, addPrefix);
}

}
