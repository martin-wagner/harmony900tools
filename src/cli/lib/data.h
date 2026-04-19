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


/** single data block */
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

    std::chrono::microseconds mark() const
    {
      return std::chrono::microseconds(mark_us);
    }
    std::chrono::microseconds segment() const
    {
      return std::chrono::microseconds(segment_us);
    }
    std::chrono::microseconds pause() const
    {
      return std::chrono::microseconds(pause_us);
    }
};

/** entire stream (single frame or actual stream) of
 * timing data */
class TimingStream
{
  protected:
    std::vector<Block> data;

  public:
    TimingStream()
    {
    }

    /** Stream lesen, Codierung Mark, Mark+Pause (=Periodendauer).
     * Wir so in irlearn verwendet */
    static TimingStream fromMarkSegment(const std::vector<uint16_t> &raw)
    {
      TimingStream ts;
      ts.addMarkSegment(raw);
      return ts;
    }

    /** Stream lesen, Codierung Mark, Pause */
    static TimingStream fromMarkPause(const std::vector<uint16_t> &raw)
    {
      TimingStream ts;
      ts.addMarkPause(raw);
      return ts;
    }

    void addMarkSegment(const std::vector<uint16_t> &raw);
    void addMarkPause(const std::vector<uint16_t> &raw);

    std::vector<uint16_t> convertMarkPause() const;
    std::string convertGnuplot(bool activeHigh = true) const;
    std::string convertHexString() const;
    std::string convertIntString() const;
    std::string convertAsciiPlot(uint32_t width = 100, bool activeHigh = true) const;

    const std::vector<Block> &timings() const
    {
      return data;
    }

};

}
