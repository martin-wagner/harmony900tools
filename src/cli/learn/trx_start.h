/*
 * start.h
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */


#pragma once

#include "trx_base.h"

namespace trx {

class Start : public Base
{
  private:
    const std::vector<uint8_t> frameReq = { 0x20, 0xA1, 0x80, 0x01, 0x01, 0x00 };

    int getHeaderMinSize() { return 4; };
    int getProtoclCmd() { return frameReq[1]; };;
    int getProtocolRespByte3()  { return 0; };

  public:
    std::vector<uint8_t> get()
    {
      return frameReq;
    }

};


}


