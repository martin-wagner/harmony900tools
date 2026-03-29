/*
 * payload.h
 *
 *  Created on: Mar 29, 2026
 *      Author: martin
 */


#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <string>

namespace frame {


/* single data block */
class Block
{
  public:
    Block(uint16_t mark_us, uint16_t segment_us) :
        mark_us(mark_us), segment_us(segment_us)
    {
    }

    const uint16_t mark_us;
    const uint16_t segment_us;

    uint16_t pause_us() const
    {
      return segment_us - mark_us;
    }

    std::chrono::microseconds mark()
    {
      return std::chrono::microseconds(mark_us);
    }
    std::chrono::microseconds segment()
    {
      return std::chrono::microseconds(segment_us);
    }
    std::chrono::microseconds pause()
    {
      return std::chrono::microseconds(pause_us());
    }
};

/* entire stream (single frame or actual stream) of
 * timing data */
class TimingStream
{
  protected:
    std::vector<Block> data;

    void setData(const std::vector<uint16_t> &raw);

  public:
    TimingStream(const std::vector<uint16_t> &raw)
    {
      setData(raw);
    }

    std::string convertGnuplot(bool activeHigh = true);
    std::string convertHexString();
    std::string convertIntString();

    const std::vector<Block> &timings()
    {
      return data;
    }

};

}
