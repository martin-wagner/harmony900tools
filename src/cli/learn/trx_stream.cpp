/*
 * stream.cpp
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */

#include "trx_stream.h"

using namespace std;

namespace trx
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

Stream::Status Stream::addChunk(const std::vector<uint8_t> &data, bool first)
{
  auto status = Base::check(data);
  if (status !=  Status::OK) {
    return status;
  }

  //data available, check remaining header + size
  if ((data.size() != VALID_CHUNK_SIZE) || (data[4] != 0x70)) {
    return Status::ERR_RESPONSE_FORMAT;
  }
  //terminator
  if ((data[data.size() - 2] != 0x01) || (data[data.size() - 1] != 0x30)) {
    return Status::ERR_TERM;
  }

  auto ret = parse( { data.begin() + 5, data.end() - 2 });
  if (ret != Status::OK) {
    return ret;
  }

  //move bytes with unknown use
  moveExcessBytes(first);

  return Status::OK;
}

}

