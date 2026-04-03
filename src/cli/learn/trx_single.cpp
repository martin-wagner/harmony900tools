/*
 * single.cpp
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */

#include "trx_single.h"

using namespace std;

namespace trx
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

Single::Status Single::addChunk(const std::vector<uint8_t> &data, bool first)
{
  auto status = Base::check(data);
  if (status !=  Status::OK) {
    return status;
  }

  //data available, check remaining header + size
  if ((data.size() != VALID_CHUNK_SIZE) || (data[4] != 0x01)) {
    return Status::ERR_RESPONSE_FORMAT;
  }

  auto ret = parse( { data.begin() + 6, data.end() });
  if (ret != Status::OK) {
    return ret;
  }

  //move bytes with unknown use
  moveExcessBytes(first);

  //check for end marker
  if (data[5] == 0) {
    return Status::DONE;
  }
  return Status::OK;
}

}
