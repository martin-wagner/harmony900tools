// SPDX-License-Identifier: LGPL-2.1-or-later

#include "trx_stream.h"

#include "lib/harmony_endian.h"

using namespace std;

namespace binary
{
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
  if (status != Status::OK) {
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

  auto ret = lib::parseHarmony16_network( { data.begin() + 5, data.end() - 2 },
      payload);
  if (ret != true) {
    return Status::ERR_SIZE;
  }

  //move non-timing words
  moveExcessBytes(first);
  //calculate clock
  if (first) {
    clock = static_cast<double>(excess[1]) * 1000000.0 / payload[0];
  }

  return Status::OK;
}

}
}
