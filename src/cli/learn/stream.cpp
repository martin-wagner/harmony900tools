/*
 * stream.cpp
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */

#include "stream.h"

using namespace std;

namespace frame
{

Stream::Status Stream::parse(const std::vector<uint8_t> &p)
{
  uint16_t v;

  if ((p.size() % 2) != 0) {
    return Status::ERR_SIZE;
  }

  for (size_t i = 0; i < p.size(); i += 2) {
    uint16_t a = p[i + 1];
    uint16_t b = p[i];
    v = a | (b << 8);
    payload.push_back(v);
  }
  return Status::OK;
}

Stream::Status Stream::addChunk(const std::vector<uint8_t> &data)
{
  if (data.size() < HEADER_MIN_SIZE) {
    return Status::ERR_SIZE;
  }
  //header
  if ((data[0] != 0x20) || (data[1] != 0xA3) || (data[2] != 0x01)
      || (data[3] != 0x02) || (data[4] != 0x70)) {
    return Status::ERR_FORMAT;
  }
  //data available, check size
  if (data.size() != VALID_CHUNK_SIZE) {
    return Status::ERR_FORMAT;
  }
  //terminator
  if ((data[data.size() - 2] != 0x01) || (data[data.size() - 1] != 0x30)) {
    return Status::ERR_TERM;
  }

  auto ret = parse( { data.begin() + 5, data.end() - 2 });
  if (ret != Status::OK) {
    return ret;
  }
  return Status::OK;
}

}

