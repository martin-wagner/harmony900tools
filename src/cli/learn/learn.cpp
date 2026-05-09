#ifdef _WIN32
  #define _WIN32_WINNT 0x0600   // Vista+ for inet_pton
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
#else
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

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

#include "cli/lib/lib.h"
#include "bin/data.h"
#include "trx_single.h"
#include "trx_start.h"
#include "trx_stop.h"
#include "trx_stream.h"

using namespace std;

const int HEADER_SIZE = 4;

// --- platform abstractions ---

#ifdef _WIN32
  using socket_t = SOCKET;
  #define INVALID_SOCKET_VAL INVALID_SOCKET
  static inline void closeSocket(socket_t s) { closesocket(s); }
#else
  using socket_t = int;
  #define INVALID_SOCKET_VAL (-1)
  static inline void closeSocket(socket_t s)
  {
    close(s);
  }
#endif

atomic<socket_t> sock(INVALID_SOCKET_VAL);

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
  socket_t s = sock.exchange(INVALID_SOCKET_VAL);
  if (s == INVALID_SOCKET_VAL) {
    return;
  }

  trx::Stop stop;
  auto tx = stop.get();

  send(s, reinterpret_cast<const char*>(tx.data()), static_cast<int>(tx.size()),
      0);

  cout << "ctrl+c detected, close connection" << endl;

  closeSocket(s);
}

bool openConnection(const string &host, uint16_t port)
{
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    cerr << "WSAStartup failed" << endl;
    return false;
  }
#endif

  //handle ctrl+c
  signal(SIGINT, handleSigint);

  socket_t s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == INVALID_SOCKET_VAL) {
    cerr << "Failed to create socket" << endl;
#ifdef _WIN32
    WSACleanup();
#endif
    return false;
  }

  sock = s;

  sockaddr_in a { };
  a.sin_family = AF_INET;
  a.sin_port = port;
  if (inet_pton(AF_INET, host.c_str(), &a.sin_addr) <= 0) {
    cerr << "Invalid IP/host address: " << host << endl;
    closeSocket(s);
    sock = INVALID_SOCKET_VAL;
    return false;
  }

  if (connect(s, reinterpret_cast<sockaddr*>(&a), sizeof(a)) < 0) {
    cerr << "Connection failed" << endl;
    closeSocket(s);
    sock = INVALID_SOCKET_VAL;
    return false;
  }
  return true;
}

bool readFrame(vector<uint8_t> &frame)
{
  uint8_t hdr[HEADER_SIZE];
  uint8_t buf[4096];

  frame.clear();

  auto bytes = recv(sock, reinterpret_cast<char*>(hdr), HEADER_SIZE,
  MSG_WAITALL);
  if (bytes != HEADER_SIZE) {
    return false;
  }
  frame.assign(hdr, hdr + HEADER_SIZE);

#ifdef _WIN32
  // MSG_DONTWAIT not available on Windows; use non-blocking mode temporarily
  u_long nonBlocking = 1;
  ioctlsocket(sock, FIONBIO, &nonBlocking);
  bytes = recv(sock, reinterpret_cast<char*>(buf), sizeof(buf), 0);
  u_long blocking = 0;
  ioctlsocket(sock, FIONBIO, &blocking);
  if (bytes == SOCKET_ERROR) {
    bytes = 0;
  }
#else
  bytes = recv(sock, buf, sizeof(buf), MSG_DONTWAIT);
#endif

  if (bytes > 0) {
    frame.insert(frame.end(), buf, buf + bytes);
  }
  return true;
}

bool sendFrame(const vector<uint8_t> &frame)
{
  auto bytes = send(sock, reinterpret_cast<const char*>(frame.data()),
      static_cast<int>(frame.size()), 0);

#ifdef _WIN32
  if (bytes == static_cast<int>(frame.size())) {
    return true;
  }
#else
  if (bytes == static_cast<ssize_t>(frame.size())) {
    return true;
  }
#endif
  return false;
}

bool pollSingleFrame(const string &file, bool firstTask, bool activeHigh = true)
{
  trx::Single single;
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

    auto ret = single.addChunk(rx, firstTask);
    if (ret == trx::Single::Status::DONE) {
      break;
    }
    switch (ret) {
      case trx::Single::Status::OK:
        break;
      case trx::Single::Status::ERR_TIMEOUT:
        cout << lib::writeTime() << "<- server returned timeout" << endl;
        return false;
      case trx::Single::Status::ERR_UNKNOWN_RETURNCODE:
        cout << "<- server returned error: "
            << to_string(single.getErrorByte(rx)) << endl;
        return false;
      case trx::Single::Status::ERR_SIZE:
        cout << "<- invalid size" << endl;
        return false;
      case trx::Single::Status::ERR_RESPONSE_FORMAT:
        cout << "<- invalid msg format" << endl;
        return false;
      case trx::Single::Status::ERR_PAYLOAD_FORMAT:
        cout << "<- invalid payload format" << endl;
        return false;
      default:
        cout << "<- unexpected: " << to_string((int) ret) << endl;
        return false;
    }
    firstTask = false;
  }

  auto payload = single.getPayload();
  if (payload.empty()) {
    cout << lib::writeTime() << "<- payload empty" << endl;
    return false;
  }
  auto timingStream = lib::TimingStream::fromMarkSegment(payload);

  if (cfg.verbosity > 0) {
    cout << "single frame (hex): " << timingStream.convertHexString() << endl;
  }
  cout << "single frame (tµs): " << timingStream.convertIntString() << endl;
  cout << timingStream.convertAsciiPlot(lib::getTerminalWidth(), activeHigh)
      << endl;

  if (cfg.verbosity > 0) {
    auto excess = single.getExcess();
    if (excess.size() > 0) {
      cout << lib::writeHex("<- excess words (hex): ", excess);
      cout << lib::writeData("<- excess words (dec): ", excess);
    }
  }
  cout << "IR Clock: " << single.getClock() << "Hz" << endl;

  if (!file.empty()) {
    //gnuplot
    auto str = timingStream.convertGnuplot(activeHigh);
    writeFile(file, str);
  }

  return true;
}

bool pollStream(const string &file, chrono::milliseconds timeout,
    bool firstTask, bool activeHigh = true)
{
  trx::Stream stream;
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

    auto ret = stream.addChunk(rx, firstTask);
    switch (ret) {
      case trx::Stream::Status::OK:
        break;
      case trx::Single::Status::ERR_TIMEOUT:
        cout << lib::writeTime() << "<- server returned timeout" << endl;
        return false;
      case trx::Stream::Status::ERR_UNKNOWN_RETURNCODE:
        cout << "<- invalid msg format" << endl;
        return false;
      case trx::Stream::Status::ERR_SIZE:
        cout << "<- invalid size" << endl;
        return false;
      case trx::Single::Status::ERR_RESPONSE_FORMAT:
        cout << "<- invalid msg format" << endl;
        return false;
      case trx::Single::Status::ERR_PAYLOAD_FORMAT:
        cout << "<- invalid payload format" << endl;
        return false;
      case trx::Stream::Status::ERR_TERM:
        cout << "<- terminator missing" << endl;
        return false;
      default:
        cout << "<- unexpected: " << to_string((int) ret) << endl;
        return false;
    }
    firstTask = false;

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
  auto timingStream = lib::TimingStream::fromMarkSegment(payload);

  if (cfg.verbosity > 0) {
    cout << "single frame (hex): " << timingStream.convertHexString() << endl;
  }
  cout << "single frame (tµs): " << timingStream.convertIntString() << endl;
  cout << timingStream.convertAsciiPlot(lib::getTerminalWidth(), activeHigh)
      << endl;

  if (cfg.verbosity > 0) {
    auto excess = stream.getExcess();
    if (excess.size() > 0) {
      cout << lib::writeHex("<- excess words (hex): ", excess);
      cout << lib::writeData("<- excess words (dec): ", excess);
    }
  }
  cout << "IR Clock: " << stream.getClock() << "Hz" << endl;

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

#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

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

  auto res = openConnection(cfg.host, htons(cfg.port));
  if (!res) {
    return EXIT_FAILURE;
  }

  if (cfg.verbosity > 0) {
    cout << "connected" << endl;
  }
  if (cfg.verbosity > 1) {
    cout << endl << "opening connection" << endl << "-------------" << endl;
  }

  trx::Start start;
  auto txStart = start.get();

  if (cfg.verbosity > 0) {
    cout << lib::writeHex("-> capture started: ", txStart);
  }

  sendFrame(txStart);
  if (!readFrame(frame)) {
    cout << "<- no confirmation, abort" << endl;
#ifdef _WIN32
    WSACleanup();
#endif
    return EXIT_FAILURE;
  }
  if (cfg.verbosity > 1) {
    cout << lib::writeHex("<- confirmation frame: ", frame);
  }

  auto statusStart = start.check(frame);
  if (statusStart != trx::Start::Status::OK) {
    cout << "<- invalid confirmation (" << (int) statusStart << "), abort"
        << endl;
#ifdef _WIN32
    WSACleanup();
#endif
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

  trx::Stop stop;
  auto txStop = stop.get();

  if (cfg.verbosity > 1) {
    cout << lib::writeHex("-> close connection: ", txStop);
  }
  sendFrame(txStop);
  frame.clear();
  if (!readFrame(frame)) {
    cout << "<- no confirmation, abort" << endl;
#ifdef _WIN32
    WSACleanup();
#endif
    return EXIT_FAILURE;
  }
  if (cfg.verbosity > 1) {
    cout << lib::writeHex("<- confirmation frame: ", frame);
  }
  auto statusStop = stop.check(frame);
  if (statusStop != trx::Stop::Status::OK) {
    cout << "<- invalid confirmation (" << (int) statusStop << "), ignore"
        << endl;
    //ignore error
  }

  closeSocket(sock);

#ifdef _WIN32
  WSACleanup();
#endif

  if (received_command) {
    cout << "command received!" << endl;
    return EXIT_SUCCESS;
  }
  if (cfg.doSingle) {
    cout << "silence..." << endl;
  }
  return EXIT_SUCCESS;
}
