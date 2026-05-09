/*
 * lib.h
 *
 *  Created on: Mar 28, 2026
 *      Author: martin
 */

#pragma once

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#include "lib/time.h"
#include "lib/hex.h"

namespace lib
{

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

inline std::string enumerateFilename(const std::string &filename, int number)
{
  std::size_t dot_pos = filename.find_last_of('.');

  // no extension found
  if (dot_pos == std::string::npos) {
    return filename + std::to_string(number);
  }

  std::string name = filename.substr(0, dot_pos);
  std::string ext = filename.substr(dot_pos);

  return name + std::to_string(number) + ext;
}

}
