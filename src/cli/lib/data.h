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

namespace lib {


/* single data block */
class Block
{
  public:
    static Block fromMarkSegment(uint16_t mark_us, uint16_t segment_us)
    {
      return Block(mark_us, segment_us, segment_us - mark_us);
    }

    static Block fromMarkPause(uint16_t mark_us, uint16_t pause_us)
    {
      return Block(mark_us, pause_us + mark_us, pause_us);
    }

  protected:
    Block(uint16_t mark_us, uint16_t segment_us, uint16_t pause_us) :
        mark_us(mark_us), pause_us(pause_us), segment_us(segment_us)
    {
    }

  public:
    const uint16_t mark_us;
    const uint16_t pause_us;
    const uint16_t segment_us;

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
      return std::chrono::microseconds(pause_us);
    }
};

/* entire stream (single frame or actual stream) of
 * timing data */
class TimingStream
{
  protected:
    std::vector<Block> data;

    TimingStream() {};
    void setDataMarkSegment(const std::vector<uint16_t> &raw);
    void setDataMarkPause(const std::vector<uint16_t> &raw);

  public:
    static TimingStream fromMarkSegment(const std::vector<uint16_t> &raw)
    {
      TimingStream ts;
      ts.setDataMarkSegment(raw);
      return ts;
    }

    static TimingStream fromMarkPause(const std::vector<uint16_t> &raw)
    {
      TimingStream ts;
      ts.setDataMarkPause(raw);
      return ts;
    }

    std::string convertGnuplot(bool activeHigh = true);
    std::string convertHexString();
    std::string convertIntString();
    std::string convertAsciiPlot(uint32_t width = 100, bool activeHigh = true);

    const std::vector<Block> &timings()
    {
      return data;
    }

};

}
