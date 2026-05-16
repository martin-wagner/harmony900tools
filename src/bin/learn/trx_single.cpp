// SPDX-License-Identifier: LGPL-2.1-or-later

#include "trx_single.h"

#include "lib/harmony_endian.h"

using namespace std;

namespace binary
{
namespace trx
{

Single::Status Single::parse(const std::vector<uint8_t> &p)
{
  //0x02 0xXX 0xXX -> size mod 3
  if ((p.size() % 3) != 0) {
    return Status::ERR_SIZE;
  }

  //drop 0x02, 0xXX 0xXX big endian
  for (size_t i = 0; (i + 2) < p.size(); i += 3) {
    if (p[i] == 0x02) {
      payload.push_back(lib::parseHarmony16_network(p[i + 1], p[i + 2]));
    } else {
      return Status::ERR_PAYLOAD_FORMAT;
    }
  }
  return Status::OK;
}

Single::Status Single::addChunk(const std::vector<uint8_t> &data, bool first)
{
  auto status = Base::check(data);
  if (status != Status::OK) {
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

  //move non-timimg words
  moveExcessBytes(first);
  //calculate clock
  if (first) {
    clock = static_cast<double>(excess[1]) * 1000000.0 / payload[0];
  }

  //check for end marker
  if (data[5] == 0) {
    return Status::DONE;
  }
  return Status::OK;
}

}
}
