// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <iomanip>

namespace lib
{


inline std::string writeTime()
{
  using namespace std::chrono;

  std::stringstream str;

  // get current time
  auto now = system_clock::now();
  auto secs = time_point_cast<seconds>(now);
  auto ms = duration_cast<milliseconds>(now - secs).count();

  // convert to calendar time
  time_t tt = system_clock::to_time_t(secs);
  tm local = *localtime(&tt);

  // print timestamp hh:mm:ss:msmsms
  str << std::setfill('0') << std::setw(2) << local.tm_hour << ":"
      << std::setw(2) << local.tm_min << ":" << std::setw(2) << local.tm_sec
      << ":" << std::setw(3) << ms << " ";

  return str.str();
}

inline void printTime(bool endl = false)
{
  std::cout << writeTime();
  if (endl) {
    std::cout << std::endl;
  }
}

}
