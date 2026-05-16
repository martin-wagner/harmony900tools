// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <vector>

namespace lib {

//use on network data stream: parseHarmony16(p[i], p[i+1])
inline uint16_t parseHarmony16_network(uint8_t h, uint8_t l)
{
  return static_cast<uint16_t>(l) | (static_cast<uint16_t>(h) << 8);
}


inline bool parseHarmony16_network(const std::vector<uint8_t> &in, std::vector<uint16_t> &out)
{
  if ((in.size() % 2) != 0) {
    return false;
  }

  for (size_t i = 0; i < in.size(); i += 2) {
    out.push_back(parseHarmony16_network(in[i], in[i+1]));
  }
  return true;
}

//use on network data stream: parseHarmony16(p[i], p[i+1])
inline uint16_t parseHarmony16_file(uint8_t h, uint8_t l)
{
  return static_cast<uint16_t>(h) | (static_cast<uint16_t>(l) << 8);
}

inline uint32_t parseHarmony32_file(uint8_t h, uint8_t m1, uint8_t m2, uint8_t l)
{
  return static_cast<uint32_t>(h) | (static_cast<uint16_t>(m1) << 8)
      | (static_cast<uint16_t>(m2) << 16) | (static_cast<uint16_t>(l) << 24);
}

inline bool parseHarmony16_file(const std::vector<uint8_t> &in, std::vector<uint16_t> &out)
{
  if ((in.size() % 2) != 0) {
    return false;
  }

  for (size_t i = 0; i < in.size(); i += 2) {
    out.push_back(parseHarmony16_file(in[i], in[i+1]));
  }
  return true;
}

inline void setHarmony16_file(uint16_t data, std::vector<uint8_t> &out)
{
  out.push_back(data);
  out.push_back(data >> 8);
}

inline void setHarmony32_file(uint32_t data, std::vector<uint8_t> &out)
{
  out.push_back(data);
  out.push_back(data >> 8);
  out.push_back(data >> 16);
  out.push_back(data >> 24);
}

inline void setHarmony16_file(const std::vector<uint16_t> &in, std::vector<uint8_t> &out)
{
  for (const auto &d : in) {
    setHarmony16_file(d, out);
  }
}


}
