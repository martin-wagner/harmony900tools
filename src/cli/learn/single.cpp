/*
 * single.cpp
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */

#include "single.h"

using namespace std;

namespace frame
{

Single::Status Single::parse(const std::vector<uint8_t> &p)
{
  uint16_t v;

  //0x02 0xXX 0xXX -> size mod 3
  if ((p.size() % 3) != 0) {
    return Status::ERR_SIZE;
  }

  //drop 0x02, 0xXX 0xXX big endian
  for (size_t i = 0; (i + 2) < p.size(); i += 3) {
    if (p[i] == 0x02) {
      uint16_t a = p[i + 2];
      uint16_t b = p[i + 1];
      v = a | (b << 8);
      payload.push_back(v);
    } else {
      return Status::ERR_PAYLOAD_FORMAT;
    }
  }
  return Status::OK;
}

Single::Status Single::addChunk(const std::vector<uint8_t> &data)
{
  if (data.size() < HEADER_MIN_SIZE) {
    return Status::ERR_SIZE;
  }
  //universal bytes of header
  if ((data[0] != 0x20) || (data[1] != 0xA2)) {
    return Status::ERR_RESPONSE_FORMAT;
  }
  //check for timeout
  if (data[2] == 0x02) {
    return Status::ERR_TIMEOUT;
  }
  //not ok
  if (data[2] != 0x01) {
    return Status::ERR_RETURN;
  }
  //data available, check remaining header + size
  if ((data.size() != VALID_CHUNK_SIZE) || (data[3] != 0x05)
      || (data[4] != 0x01)) {
    return Status::ERR_RESPONSE_FORMAT;
  }

  auto ret = parse( { data.begin() + 6, data.end() });
  if (ret != Status::OK) {
    return ret;
  }
  //check for end marker
  if (data[5] == 0) {
    return Status::DONE;
  }
  return Status::OK;
}

uint8_t Single::getError(const std::vector<uint8_t> &data)
{
  if (data.size() < 3) {
    return 255;
  }
  return data[2];
}

}
