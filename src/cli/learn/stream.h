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

//see learn.md _read single frame_
class Stream
{
  public:

    enum class Status
    {
      OK,
      ERR_SIZE,
      ERR_FORMAT,
      ERR_TERM,
    };

  private:
    const std::vector<uint8_t> frameReq = { 0x20, 0xA3, 0x80, 0x00 };
    const int HEADER_MIN_SIZE = 5;
    const int VALID_CHUNK_SIZE = 199;
    std::vector<uint16_t> payload;


    Status parse(const std::vector<uint8_t> &p);

  public:
    Stream()
    {
    }

    std::vector<uint8_t> get()
    {
      return frameReq;
    }

    Status addChunk(const std::vector<uint8_t> &data);

    std::vector<uint16_t> getPayload()
    {
      return payload;
    }

    void clearPayload()
    {
      payload.clear();
    }
};


}


