/*
 * start.h
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */


#pragma once

#include <cstdint>
#include <vector>

namespace frame {

class Stop
{
  private:
    const std::vector<uint8_t> frameReq = { 0x20, 0xA4, 0x80, 0x00 };
    const std::vector<uint8_t> frameConfirm = { 0x20, 0xA4, 0x01, 0x00 };

  public:
    Stop()
    {
    }

    std::vector<uint8_t> get()
    {
      return frameReq;
    }

    bool check(const std::vector<uint8_t> &data)
    {
      if (data == frameConfirm) {
        return true;
      }
      return false;
    }

};


}


