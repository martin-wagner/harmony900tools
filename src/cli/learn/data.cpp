/*
 * data.cpp
 *
 *  Created on: Mar 29, 2026
 *      Author: martin
 */

#include <sstream>
#include <iomanip>

#include "data.h"

using namespace std;

namespace frame
{

void TimingStream::setData(const vector<uint16_t> &raw)
{
  for (size_t i = 0; i < raw.size(); i += 2) {
    if (i + 1 < raw.size()) {
      data.push_back(Block(raw[i], raw[i + 1]));
    }
  }
}

string TimingStream::convertGnuplot(bool activeHigh)
{
  vector<pair<uint32_t, int>> plotData;
  uint32_t time_us = 0;
  stringstream str;

  uint32_t markValue = 1;
  uint32_t pauseValue = 0;
  if (!activeHigh) {
    markValue = 0;
    pauseValue = 1;
  }

  for (const auto &block : data) {
    // Start of mark (ON state)
    plotData.push_back( { time_us, markValue });

    // End of mark, start of segment (OFF state)
    time_us += block.mark_us;
    plotData.push_back( { time_us, 0 });

    // End of segment (prepare for next mark)
    time_us += block.pause_us();
  }

  str << "time(µs) mark" << endl;
  for (const auto& [time, amp] : plotData) {
    str << time << " " << amp << endl;
  }
  return str.str();
}

std::string TimingStream::convertHexString()
{
  std::stringstream str;

  for (const auto &block : data) {
    str << std::hex << std::setw(4) << std::setfill('0') << block.mark_us << " "
        << std::hex << std::setw(4) << std::setfill('0') << block.segment_us; //raw data, not mark/pause!
  }
  str << std::dec;

  return str.str();
}

std::string TimingStream::convertIntString()
{
  std::stringstream str;

  for (const auto &block : data) {
    str << "MP" << block.mark_us << ":" << block.pause_us() << "; ";
  }

  return str.str();
}

}
