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

class Start
{
  private:
    const std::vector<uint8_t> frameReq = { 0x20, 0xA1, 0x80, 0x01, 0x01, 0x00 };
    const std::vector<uint8_t> frameConfirm = { 0x20, 0xA1, 0x01, 0x00 };

  public:
    Start()
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


