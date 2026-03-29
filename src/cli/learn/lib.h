/*
 * lib.h
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */

#pragma once

#include <iostream>
#include <chrono>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif



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

//string hh:mm:ss:MMM text 00 00 00 ...
inline std::string writeHex(const std::string &text, const std::vector<uint8_t> &data)
{
  std::stringstream str;

  str << writeTime() << text;

  for (auto b : data) {
    str << std::hex << std::setw(2) << std::setfill('0') << (int) b << " ";
  }
  str << std::dec << std::endl;

  return str.str();
}

//string hh:mm:ss:MMM text 0000 0000 0000 ...
inline std::string writeHex(const std::string &text, const std::vector<uint16_t> &data)
{
  std::stringstream str;

  str << writeTime() << text;

  for (auto w : data) {
    str << std::hex << std::setw(4) << std::setfill('0') << (int) w << " ";
  }
  str << std::dec << std::endl;

  return str.str();
}

//string hh:mm:ss:MMM text 00000 00000 00000 ...
inline std::string writeData(const std::string &text, const std::vector<uint16_t> &data)
{
  std::stringstream str;

  str << writeTime() << text;

  for (auto w : data) {
    str << std::setw(5) << std::setfill('0') << (int) w << " ";
  }
  str << std::endl;

  return str.str();
}

//terminal width in chars
inline int getTerminalWidth()
{
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
    return w.ws_col;
  }
#endif
  return 80;
}

}
