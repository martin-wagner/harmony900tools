// SPDX-License-Identifier: LGPL-2.1-or-later

#include "trx_base.h"

using namespace std;

namespace binary {
namespace trx
{

Base::Status Base::check(const std::vector<uint8_t> &data)
{
  if (data.size() < getHeaderMinSize()) {
    return Status::ERR_SIZE;
  }
  if ((data[0] != 0x20) || (data[1] != getProtoclCmd())
      || (data[3] != getProtocolRespByte3())) {
    return Status::ERR_RESPONSE_FORMAT;
  }
  switch (static_cast<ProtocolStatus>(data[2])) {
    case ProtocolStatus::OK:
      return Status::OK;
    case ProtocolStatus::TIMEOUT:
      return Status::ERR_TIMEOUT;
    default:
      return Status::ERR_UNKNOWN_RETURNCODE;
  }
}

uint8_t Base::getErrorByte(const std::vector<uint8_t> &data)
{
  if (data.size() < getHeaderMinSize()) {
    return 255;
  }
  return data[2];
}

void Base::moveExcessBytes(bool check)
{
  if (payload.size() < 3) {
    return;
  }

  if (check) {
    excess.push_back(payload[0]);
    excess.push_back(payload[2]);
    payload.erase(payload.begin() + 2);
    payload.erase(payload.begin() + 0);
  }
}

}
}
