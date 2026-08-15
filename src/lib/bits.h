// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <stdint.h>
#include <vector>

namespace lib
{

// LSB first in vector (data[0] = bit 0)
inline uint64_t bitsTou64Lsb(const std::vector<bool> &data)
{
  uint64_t value = 0;
  size_t i;

  for (i = 0; i < data.size(); i++) {
    if (data[i] == true) {
      value = value | (static_cast<uint64_t>(0x01) << i);
    }
  }
  return value;
}

// MSB first in vector (data[0] = highest bit)
inline uint64_t bitsTou64Msb(const std::vector<bool> &data)
{
  uint64_t value = 0;

  for (auto bit : data) {
    value = value << 1;
    if (bit == true) {
      value = value | 0x01;
    }
  }
  return value;
}

// LSB first in vector (bits[0] = bit 0)
inline std::vector<bool> u64ToBitsLsb(uint8_t bitCount, uint64_t data)
{
  int i;

  std::vector<bool> bits;
  bits.reserve(bitCount);

  for (i = 0; i < bitCount; i++) {
    if ((data & 0x01) != 0) {
      bits.push_back(true);
    } else {
      bits.push_back(false);
    }
    data = data >> 1;
  }
  return bits;
}

// MSB first in vector (bits[0] = highest of the bitCount bits)
inline std::vector<bool> u64ToBitsMsb(uint8_t bitCount, uint64_t data)
{
  int i;

  std::vector<bool> bits;
  bits.reserve(bitCount);

  for (i = bitCount - 1; i >= 0; i--) {
    if (((data >> i) & 0x01) != 0) {
      bits.push_back(true);
    } else {
      bits.push_back(false);
    }
  }
  return bits;
}

inline uint8_t reverseBits(uint8_t v)
{
    v = ((v & 0x55) << 1) | ((v >> 1) & 0x55);
    v = ((v & 0x33) << 2) | ((v >> 2) & 0x33);
    return (v << 4) | (v >> 4);
}

}
