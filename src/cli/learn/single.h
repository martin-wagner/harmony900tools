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
class Single
{
  public:

    enum class Status
    {
      OK,
      DONE,
      ERR_SIZE,
      ERR_RETURN,
      ERR_RESPONSE_FORMAT,
      ERR_PAYLOAD_FORMAT,
      ERR_TIMEOUT
    };

  private:
    const std::vector<uint8_t> frameReq = { 0x20, 0xA2, 0x80, 0x00 };
    const int HEADER_MIN_SIZE = 4;
    const int VALID_CHUNK_SIZE = 18;
    std::vector<uint16_t> payload;

    Status parse(const std::vector<uint8_t> &p);

  public:
    Single()
    {
    }

    std::vector<uint8_t> get()
    {
      return frameReq;
    }

    Status addChunk(const std::vector<uint8_t> &data);
    uint8_t getError(const std::vector<uint8_t> &data);

    std::vector<uint16_t> getPayload()
    {
      std::vector<uint16_t> ret;

      if (payload.size() < 4) {
        return ret;
      }

      //word 0 and 2 are not timing values -- ???
      //manually copy first pair
      ret.push_back(payload[1]);
      ret.push_back(payload[3]);

      ret.insert(ret.end(), payload.begin() + 4, payload.end());
      return ret;
    }

    std::vector<uint16_t> getQ()
    {
      std::vector<uint16_t> ret;

      if (payload.size() < 4) {
        return ret;
      }

      //word 0 and 2 are not timing values -- ???
      ret.push_back(payload[0]);
      ret.push_back(payload[2]);
      return ret;
    }

    void clearPayload()
    {
      payload.clear();
    }
};


}


