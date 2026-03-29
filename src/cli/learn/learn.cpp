#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <csignal>
#include <atomic>
#include <getopt.h>

#include "lib.h"
#include "start.h"
#include "single.h"
#include "stream.h"
#include "stop.h"
#include "data.h"

using namespace std;

const int HEADER_SIZE = 4;

atomic<int> sock = -1;

// --- config ---

struct Config
{
    int verbosity = 0;
    string host = "169.254.1.2";
    uint16_t port = 3074;
    string fileSingle = "frame_ir.dat";
    string fileStream = "streaming_ir.dat";
    bool doSingle = true;
    bool doStream = false;
    int streamSeconds = 5;
    bool activeHigh = true;
} cfg;

static void printHelp(const char *prog)
{
  cout << "Usage: " << prog << " [options]\n" << "\n" << "Options:\n"
      << "  -h             Show this help\n"
      << "  -v             Verbose (repeat for -vv)\n"
      << "  -i <ip>        IP address         (default: 169.254.1.2)\n"
      << "  -p <port>      Port               (default: 3074)\n"
      << "  -f <file>      Single-frame file  (default: frame_ir.dat)\n"
      << "  -F <file>      Stream file        (default: streaming_ir.dat)\n"
      << "  -s[0|1]        Poll single frame  (default: on,  -s0 disables)\n"
      << "  -S[0|1]        Poll stream        (default: off, -S1 enables)\n"
      << "  -t <seconds>   Stream duration    (default: 5)\n"
      << "  -l             Active-low signal  (default: active-high)\n" << "\n"
      << "At least one of -s/-S (or their defaults) must be active.\n";
}

static bool parseArgs(int argc, char **argv, Config &cfg)
{
  int opt;
  while ((opt = getopt(argc, argv, "hvi:p:f:F:s::S::t:l")) != -1) {
    switch (opt) {
      case 'h':
        return false;
      case 'v':
        cfg.verbosity++;
        break;
      case 'i':
        cfg.host = optarg;
        break;
      case 'p': {
        int p = atoi(optarg);
        if (p <= 0 || p > 65535) {
          cerr << "Invalid port: " << optarg << endl;
          return false;
        }
        cfg.port = static_cast<uint16_t>(p);
        break;
      }
      case 'f':
        cfg.fileSingle = optarg;
        break;
      case 'F':
        cfg.fileStream = optarg;
        break;
      case 's': {
        // -s or -s1 enables, -s0 disables
        if (optarg != nullptr && string(optarg) == "0") {
          cfg.doSingle = false;
        } else {
          cfg.doSingle = true;
        }
        break;
      }
      case 'S': {
        if (optarg != nullptr && string(optarg) == "0") {
          cfg.doStream = false;
        } else {
          cfg.doStream = true;
        }
        break;
      }
      case 't': {
        int t = atoi(optarg);
        if (t <= 0) {
          cerr << "Invalid stream duration: " << optarg << endl;
          return false;
        }
        cfg.streamSeconds = t;
        break;
      }
      case 'l':
        cfg.activeHigh = false;
        break;
      default:
        return false;
    }
  }

  if (!cfg.doSingle && !cfg.doStream) {
    cerr << "Error: at least one of single frame or stream must be enabled."
        << endl;
    return false;
  }

  return true;
}

bool writeFile(const string &filename, const string &data)
{
  ofstream file(filename);

  if (!file.is_open()) {
    return false;
  }

  file << data;

  file.close();

  return true;
}

void handleSigint(int)
{
  int s = sock.exchange(-1);
  if (s < 0) {
    return;
  }

  frame::Stop stop;
  auto tx = stop.get();

  send(s, tx.data(), tx.size(), 0);

  cout << "ctrl+c detected, close connection" << endl;

  close(s);
}

bool open(const string &host, uint16_t port)
{
  //handle ctrl+c
  signal(SIGINT, handleSigint);

  sock = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in a { };
  a.sin_family = AF_INET;
  a.sin_port = port;
  if (inet_pton(AF_INET, host.c_str(), &a.sin_addr) <= 0) {
    cerr << "Invalid IP/host address: " << host << endl;
    close(sock);
    return false;
  }

  if (connect(sock, (sockaddr*) &a, sizeof(a)) < 0) {
    cerr << "Connection failed" << endl;
    close(sock);
    return false;
  }
  return true;
}

bool readFrame(vector<uint8_t> &frame)
{
  uint8_t hdr[HEADER_SIZE];
  uint8_t buf[4096];

  frame.clear();

  auto bytes = recv(sock, hdr, HEADER_SIZE, MSG_WAITALL);
  if (bytes != HEADER_SIZE) {
    return false;
  }
  frame.assign(hdr, hdr + HEADER_SIZE);

  bytes = recv(sock, buf, sizeof(buf), MSG_DONTWAIT);
  if (bytes > 0) {
    frame.insert(frame.end(), buf, buf + bytes);
  }
  return true;
}

bool sendFrame(const vector<uint8_t> &frame)
{
  auto bytes = send(sock, frame.data(), frame.size(), 0);
  if (bytes == frame.size()) {
    return true;
  }
  return false;
}

bool pollSingleFrame(const string &file, bool activeHigh = true)
{
  frame::Single single;
  vector<uint8_t> rx;

  if (cfg.verbosity > 0) {
    cout << "poll single frame" << endl << "-------------" << endl;
  }

  while (true) {
    auto tx = single.get();
    if (cfg.verbosity > 0) {
      cout << lib::writeHex("-> poll chunk: ", tx);
    }
    sendFrame(tx);

    if (!readFrame(rx)) {
      cout << "<- no/error data rx, abort" << endl;
      break;
    }

    if (cfg.verbosity > 1) {
      cout << lib::writeHex("<- rx chunk: ", rx);
    }

    auto ret = single.addChunk(rx);
    if (ret == frame::Single::Status::DONE) {
      break;
    }
    switch (ret) {
      case frame::Single::Status::OK:
        break;
      case frame::Single::Status::ERR_TIMEOUT:
        cout << lib::writeTime() << "<- server returned timeout" << endl;
        return false;
      case frame::Single::Status::ERR_RETURN:
        cout << "<- server returned error: " << to_string(single.getError(rx))
            << endl;
        return false;
      case frame::Single::Status::ERR_SIZE:
        cout << "<- invalid size" << endl;
        return false;
      case frame::Single::Status::ERR_RESPONSE_FORMAT:
        cout << "<- invalid msg format" << endl;
        return false;
      case frame::Single::Status::ERR_PAYLOAD_FORMAT:
        cout << "<- invalid payload format" << endl;
        return false;
      default:
        cout << "<- unexpected: " << to_string((int) ret) << endl;
        return false;
    }
  }

  auto payload = single.getPayload();
  if (payload.empty()) {
    cout << lib::writeTime() << "<- payload empty" << endl;
    return false;
  }
  frame::TimingStream timingStream(payload);

  if (cfg.verbosity > 0) {
    cout << "single frame (hex): " << timingStream.convertHexString() << endl;
  }
  cout << "single frame (tµs): " << timingStream.convertIntString() << endl;
  cout << timingStream.convertAsciiPlot(lib::getTerminalWidth(), activeHigh)
      << endl;

  //todo find what those two words do. might have something to do with the corresponding signal (??)
  if (cfg.verbosity > 0) {
    auto leftover = single.getQ();
    cout << lib::writeHex("<- leftover words (hex): ", leftover);
    cout << lib::writeData("<- leftover words (dec): ", leftover);
  }

  if (!file.empty()) {
    //gnuplot
    auto str = timingStream.convertGnuplot(activeHigh);
    writeFile(file, str);
  }

  return true;
}

bool pollStream(const string &file, chrono::milliseconds timeout,
    bool activeHigh = true)
{
  frame::Stream stream;
  vector<uint8_t> rx;

  if (cfg.verbosity > 0) {
    cout << "poll stream" << endl << "-------------" << endl;
  }

  auto t_start = chrono::steady_clock::now();

  while (true) {
    auto tx = stream.get();
    if (cfg.verbosity > 0) {
      cout << lib::writeHex("-> poll chunk: ", tx);
    }
    sendFrame(tx);

    if (!readFrame(rx)) {
      cout << "<- no/error data rx, abort" << endl;
      break;
    }

    if (cfg.verbosity > 1) {
      cout << lib::writeHex("<- rx chunk: ", rx);
    }

    auto ret = stream.addChunk(rx);
    switch (ret) {
      case frame::Stream::Status::OK:
        break;
      case frame::Stream::Status::ERR_SIZE:
        cout << "<- invalid size" << endl;
        return false;
      case frame::Stream::Status::ERR_FORMAT:
        cout << "<- invalid msg format" << endl;
        return false;
      case frame::Stream::Status::ERR_TERM:
        cout << "<- terminator missing" << endl;
        return false;
      default:
        cout << "<- unexpected: " << to_string((int) ret) << endl;
        return false;
    }

    auto t_now = chrono::steady_clock::now();
    if ((t_now - t_start) > timeout) {
      break;
    }
  }

  auto payload = stream.getPayload();
  if (payload.empty()) {
    cout << lib::writeTime() << "<- payload empty" << endl;
    return false;
  }
  frame::TimingStream timingStream(payload);

  if (cfg.verbosity > 0) {
    cout << "single frame (hex): " << timingStream.convertHexString() << endl;
  }
  cout << "single frame (tµs): " << timingStream.convertIntString() << endl;
  cout << timingStream.convertAsciiPlot(lib::getTerminalWidth(), activeHigh)
      << endl;

  if (!file.empty()) {
    //gnuplot
    auto str = timingStream.convertGnuplot(activeHigh);
    writeFile(file, str);
  }

  return true;
}

int main(int argc, char **argv)
{
  vector<uint8_t> frame;
  bool received_command = false;

  if (!parseArgs(argc, argv, cfg)) {
    printHelp(argv[0]);
    return EXIT_FAILURE;
  }

  if (cfg.verbosity >= 1) {
    cout << "host=" << cfg.host << " port=" << cfg.port << " single="
        << cfg.doSingle << " stream=" << cfg.doStream << " streamSec="
        << cfg.streamSeconds << " activeHigh=" << cfg.activeHigh
        << " verbosity=" << cfg.verbosity << endl;
  }

  auto res = open(cfg.host, htons(cfg.port));
  if (!res) {
    return EXIT_FAILURE;
  }

  if (cfg.verbosity > 0) {
    cout << "connected" << endl;
  }
  if (cfg.verbosity > 1) {
    cout << endl << "opening connection" << endl << "-------------" << endl;
  }

  frame::Start start;
  auto txStart = start.get();

  if (cfg.verbosity > 0) {
    cout << lib::writeHex("-> capture started: ", txStart);
  }

  sendFrame(txStart);
  if (!readFrame(frame)) {
    cout << "<- no confirmation, abort" << endl;
    return EXIT_FAILURE;
  }
  if (cfg.verbosity > 1) {
    cout << lib::writeHex("<- confirmation frame: ", frame);
  }

  res = start.check(frame);
  if (!res) {
    cout << "<- invalid confirmation, abort" << endl;
    return EXIT_FAILURE;
  }

  cout << "press remote" << endl;

  if (cfg.doSingle) {
    received_command = pollSingleFrame(cfg.fileSingle, cfg.activeHigh);
  }

  if (cfg.doStream) {
    pollStream(cfg.fileStream, chrono::seconds(cfg.streamSeconds),
        cfg.activeHigh);
  }

  if (cfg.verbosity > 0) {
    cout << "closing connection" << endl << "-------------" << endl;
  }

  frame::Stop stop;
  auto txStop = stop.get();

  if (cfg.verbosity > 1) {
    cout << lib::writeHex("-> close connection: ", txStop);
  }
  sendFrame(txStop);
  frame.clear();
  if (!readFrame(frame)) {
    cout << "<- no confirmation, abort" << endl;
    return 1;
  }
  if (cfg.verbosity > 1) {
    cout << lib::writeHex("<- confirmation frame: ", frame);
  }
  //ignore data

  close(sock);

  if (received_command) {
    cout << "command received!" << endl;
    return 0;
  }
  if (cfg.doSingle) {
    cout << "silence..." << endl;
  }
  return 0;
}
