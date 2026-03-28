//https://chatgpt.com/c/69b6d342-05b4-832a-a0fb-15bedba19922

#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <csignal>
#include <atomic>

#include "ir_parser1.cpp"

using namespace std;
using namespace std::literals;
// enables literal suffixes, e.g. 24h, 1ms, 1s.

volatile int s = -1;

void handle_sigint(int)
{
  auto s_l = s;
  s = -1;
  if (s_l < 0) {
    return;
  }

  vector<uint8_t> stop = { 0x20, 0xA4, 0x80, 0x00 };
  send(s_l, stop.data(), stop.size(), 0);

  cout << "ctrl+c detected, close connection" << endl;

  close(s_l);
}

void printTime()
{
  using namespace std::chrono;

  // get current time
  auto now = system_clock::now();
  auto secs = time_point_cast<seconds>(now);
  auto ms = duration_cast<milliseconds>(now - secs).count();

  // convert to calendar time
  time_t tt = system_clock::to_time_t(secs);
  tm local = *localtime(&tt);

  // print timestamp hh:mm:ss:msmsms
  cout << setfill('0') << setw(2) << local.tm_hour << ":" << setw(2)
      << local.tm_min << ":" << setw(2) << local.tm_sec << ":" << setw(3) << ms
      << " ";
}

void dumpHex(const string &text, const vector<uint8_t> &data)
{
  printTime();

  cout << text;
  for (auto b : data) {
    cout << hex << setw(2) << setfill('0') << (int) b << " ";
  }
  cout << dec << endl;
}

void dumpData(const string &text, const vector<uint16_t> &data, bool useHex =
    true)
{
  printTime();

  cout << text;

  if (useHex) {
    for (auto b : data) {
      cout << hex << setw(4) << setfill('0') << (int) b << " ";
    }
    cout << dec << endl;
  } else {
    for (auto b : data) {
      cout << setw(5) << setfill('0') << (int) b << " ";
    }
    cout << endl;
  }
}

bool readFrame(int sock, vector<uint8_t> &frame)
{
  uint8_t hdr[4];

  if (recv(sock, hdr, 4, MSG_WAITALL) != 4)
    return false;

  frame.assign(hdr, hdr + 4);

  uint8_t buf[4096];

  int r = recv(sock, buf, sizeof(buf), MSG_DONTWAIT);

  if (r > 0)
    frame.insert(frame.end(), buf, buf + r);

  return true;
}

vector<uint16_t> parsePayload(const vector<uint8_t> &p, int byteorder = 0)
{
  vector<uint16_t> d;

  for (size_t i = 0; i + 2 < p.size(); i++) {
    if (p[i] == 0x02) {
      uint16_t v;

      if (byteorder == 0) {
        v = p[i + 1] | (p[i + 2] << 8);
      } else if (byteorder == 1) {
        v = p[i + 2] | (p[i + 1] << 8);
      } else {
        v = 0;
      }
      d.push_back(v);
      i += 2;
    } else {
      cout << "decode error at " << to_string(i) << ": ";
      cout << hex << setw(2) << setfill('0') << (int) p[i] << dec
          << ", expecting 0x02.  aborting." << endl;
      return d;
    }
  }

  return d;
}

vector<uint16_t> parseWords(const vector<uint8_t> &p, int byteorder = 0)
{
  vector<uint16_t> d;

  if (p.size() % 2 != 0) {
    cout << "decode error data not mod 2" << endl;
    return d;
  }

  for (size_t i = 0; i < p.size(); i += 2) {
    uint16_t v;

    if (byteorder == 0) {
      v = p[i] | (p[i + 1] << 8);
    } else if (byteorder == 1) {
      v = p[i + 1] | (p[i] << 8);
    } else {
      v = 0;
    }

    d.push_back(v);
  }

  return d;
}

bool isIdle(const vector<uint8_t> &d)
{
  if (d.size() < 6) {
    return false;
  }

  if (d[5] != 0) {
    return false;
  }
  return true;

//    for(auto v:d)
//        if(v>400 && v<30000)
//            return false;
//    return true;
}

bool isEnd(const vector<uint8_t> &d)
{
  if (d.size() < 2) {
    return false;
  }

  uint16_t terminator = d[d.size() - 2] << 8 | d[d.size() - 1];
  if (terminator == 0x0130) {
    return true;
  }
  return false;
}

bool poll_short_data(vector<uint16_t> &data, bool return_on_rx = false)
{
  vector<uint8_t> frame;
  bool have_data = false;

  cout << "poll single frame" << endl << "-------------" << endl;

  while (true) {
    vector<uint8_t> poll = { 0x20, 0xA2, 0x80, 0x00 };
    dumpHex("-> poll sections: ", poll);
    send(s, poll.data(), poll.size(), 0);

    frame.clear();

    if (!readFrame(s, frame)) {
      cout << "<- no/error data rx, abort" << endl;
      break;
    }

    dumpHex("<- section frame: ", frame);

    if (frame.size() < 4) {
      cout << "<- invalid size, dropping" << endl;
      continue;
    }
    if (frame[2] == 0x02) {
      printTime();
      cout << "<- timeout" << endl;
      return false;
    }
    if (frame[2] != 0x01) {
      printTime();
      cout << "<- fehler: " << to_string(frame[3]) << endl;
      return false;
    }
    //todo check header valid

    vector<uint8_t> payload(frame.begin() + 6, frame.end());
    //auto d=parsePayload(payload, 0);
    //dumpData("<- section frame (a): ",d);
    //auto d1=parsePayload(payload, 1);
    //dumpData("<- section frame (b): ",d1);
    auto d = parsePayload(payload, 1);
    dumpData("<- section frame (hex): ", d, true);
    dumpData("<- section frame (dec): ", d, false);

    data.insert(data.end(), d.begin(), d.end());

    //byte 6 = 0 -- no more data
    if (isIdle(frame)) {
      cout << "<> idle detected -> end capture" << endl;
      break;
    }
    have_data = true;

    if (return_on_rx) {
      break;
    }
  }

  return have_data;
}

void poll_long_data(vector<uint16_t> &pool)
{
  vector<uint8_t> frame;

  cout << "poll stream data" << endl << "-------------" << endl;

  vector<uint8_t> poll = { 0x20, 0xA3, 0x80, 0x00 };
  dumpHex("-> poll data: ", poll);
  send(s, poll.data(), poll.size(), 0);

  if (!readFrame(s, frame)) {
    cout << "<- no/error data rx, abort" << endl;
    return;
  }

  //endekennung 0x0130
  if (!isEnd(frame)) {
    cout << "<> endekennung fehlt" << endl;
    return;
  }

  dumpHex("<- data frame: ", frame);

  //endekennung entfernen
  frame.pop_back();
  frame.pop_back();

  vector<uint8_t> payload(frame.begin() + 5, frame.end());
  //auto d=parsePayload(payload, 0);
  //dumpData("<- section frame (a): ",d);
  //auto d1=parseWords(payload, 1);
  //dumpData("<- data frame (b): ",d1);
  auto d = parseWords(payload, 1);
  dumpData("<- section frame (hex): ", d, true);
  dumpData("<- section frame (dec): ", d, false);

  pool.insert(pool.end(), d.begin(), d.end());
}

int main(int argc, char **argv)
{
  vector<uint8_t> frame;
  vector<uint16_t> data;
  bool received_command = false;

  if (argc != 3) {
    cout << "usage: ircap ip port" << endl;
    return 0;
  }

  //handle ctrl+c
  std::signal(SIGINT, handle_sigint);

  s = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in a { };
  a.sin_family = AF_INET;
  a.sin_port = htons(atoi(argv[2]));
  if (inet_pton(AF_INET, argv[1], &a.sin_addr) <= 0) {
    perror("Invalid IP address");
    close(s);
    return 1;
  }

  if (connect(s, (sockaddr*) &a, sizeof(a)) < 0) {
    perror("Connection failed");
    close(s);
    return 1;
  }

  cout << "connected" << endl;

  cout << endl << "opening connection" << endl << "-------------" << endl;

  vector<uint8_t> start = { 0x20, 0xA1, 0x80, 0x01, 0x01, 0x00 };
  dumpHex("-> capture started: ", start);
  send(s, start.data(), start.size(), 0);
  if (!readFrame(s, frame)) {
    cout << "<- no confirmation, abort" << endl;
    return 1;
  }
  dumpHex("<- confirmation frame: ", frame);

  cout << "press remote" << endl;

  auto t_start = chrono::steady_clock::now();

  auto have_data = poll_short_data(data, false);
  if (have_data) {
    dumpData("<- single frame (hex): ", data, true);
    dumpData("<- single frame (dec): ", data, false);

    //gnuplot
    auto f = parser::parse_single_frame_mode(data);
    auto plot_data_frame = parser::to_gnuplot_frame(f);
    parser::write_gnuplot_data("frame_ir.dat", plot_data_frame);

    data.clear();

    while (true) {
      poll_long_data(data);
      received_command = true;

      auto t_now = chrono::steady_clock::now();
      if ((t_now - t_start) > 5s) {
        break;
      }
    }
  }

  if (!data.empty()) {
    dumpData("<- full stream (hex): ", data, true);
    dumpData("<- full stream (dec): ", data, false);

    //gnuplot
    auto stream = parser::parse_streaming_mode(data);
    auto plot_data = parser::to_gnuplot_streaming(stream);
    parser::write_gnuplot_data("streaming_ir.dat", plot_data);
  }

  cout << "closing connection" << endl << "-------------" << endl;

  vector<uint8_t> stop = { 0x20, 0xA4, 0x80, 0x00 };
  dumpHex("-> close connection: ", stop);
  send(s, stop.data(), stop.size(), 0);
  frame.clear();
  if (!readFrame(s, frame)) {
    cout << "<- no confirmation, abort" << endl;
    return 1;
  }
  dumpHex("<- confirmation frame: ", frame);

  close(s);

  if (received_command) {
    cout << "command received!" << endl;
    return 0;
  }
  cout << "silence..." << endl;
  return 0;
}
