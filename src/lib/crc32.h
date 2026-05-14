// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <vector>
#include <cstdint>
#include <boost/crc.hpp>

namespace lib
{

inline uint32_t calcCrc32(const std::vector<uint8_t> &data)
{
  boost::crc_32_type result;
  result.process_bytes(data.data(), data.size());
  return result.checksum();
}

}
