// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "trx_base.h"

namespace trx {

//see learn.md _read stream_
class Stream : public Base
{
  public:

  private:
    const std::vector<uint8_t> frameReq = { 0x20, 0xA3, 0x80, 0x00 };

    int getHeaderMinSize() { return 5; };
    int getProtoclCmd() { return frameReq[1]; };;
    int getProtocolRespByte3()  { return 2; };

    const int VALID_CHUNK_SIZE = 199;

    Status parse(const std::vector<uint8_t> &p);

  public:
    std::vector<uint8_t> get()
    {
      return frameReq;
    }

    Status addChunk(const std::vector<uint8_t> &data, bool first) override;

};


}


