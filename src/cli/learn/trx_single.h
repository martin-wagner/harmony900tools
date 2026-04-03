/*
 * start.h
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */


#pragma once

#include "trx_base.h"

namespace trx {

//see learn.md _read single frame_
class Single : public Base
{
  public:

  private:
    const std::vector<uint8_t> frameReq = { 0x20, 0xA2, 0x80, 0x00 };

    int getHeaderMinSize() { return 5; };
    int getProtoclCmd() { return frameReq[1]; };;
    int getProtocolRespByte3()  { return 5; };

    const int VALID_CHUNK_SIZE = 18;

    Status parse(const std::vector<uint8_t> &p);

  public:
    std::vector<uint8_t> get()
    {
      return frameReq;
    }

    Status addChunk(const std::vector<uint8_t> &data, bool first) override;
};


}


