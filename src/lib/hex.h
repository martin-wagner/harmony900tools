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

#include "lib/time.h"

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

}
