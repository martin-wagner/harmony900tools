/*
 * crc32.h
 *
 *  Created on: Apr 4, 2026
 *      Author: martin
 */


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
