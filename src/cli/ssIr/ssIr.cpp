//#include <arpa/inet.h>
//#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
//#include <chrono>
//#include <iomanip>
//#include <cstring>
//#include <cmath>
//#include <algorithm>
//#include <csignal>
//#include <atomic>
#include <getopt.h>

#include "cli/lib/lib.h"
#include "file.h"
//#include "data.h"
//#include "trx_single.h"
//#include "trx_start.h"
//#include "trx_stop.h"
//#include "trx_stream.h"
//
using namespace std;
//
//const int HEADER_SIZE = 4;
//
//atomic<int> sock = -1;
//
//// --- config ---
//
struct Config
{
    int verbosity = 0;
//    string host = "169.254.1.2";
//    uint16_t port = 3074;
    string ssIrFileName = "SsIr.bin";
    string dumpFileName = "streamIr.dat";
//    string fileStream = "streaming_ir.dat";
//    bool doSingle = true;
//    bool doStream = false;
//    int streamSeconds = 5;
    bool activeHigh = true;
} cfg;

static void printHelp(const char *prog)
{
  cout << "Usage: " << prog << " [options]\n" << "\n" << "Options:\n"
      << "  -h             Show this help\n"
      << "  -v             Verbose (repeat for -vv)\n"
//      << "  -i <ip>        IP address         (default: 169.254.1.2)\n"
//      << "  -p <port>      Port               (default: 3074)\n"
      << "  -f <file>      File to parse        (default: SsIr.bin)\n"
      << "  -d <file>      File to dump output  (default: streamIr.dat, will be numbered)\n"
//      << "  -F <file>      Stream file        (default: streaming_ir.dat)\n"
//      << "  -s[0|1]        Poll single frame  (default: on,  -s0 disables)\n"
//      << "  -S[0|1]        Poll stream        (default: off, -S1 enables)\n"
//      << "  -t <seconds>   Stream duration    (default: 5)\n"
      << "  -l             Active-low signal  (default: active-high)\n" << "\n"
//      << "At least one of -s/-S (or their defaults) must be active.\n"
      ;
}

static bool parseArgs(int argc, char **argv, Config &cfg)
{
  int opt;
  while ((opt = getopt(argc, argv, "hvi:p:f:d:F:s::S::t:l")) != -1) {
    switch (opt) {
      case 'h':
        return false;
      case 'v':
        cfg.verbosity++;
        break;
//      case 'i':
//        cfg.host = optarg;
//        break;
//      case 'p': {
//        int p = atoi(optarg);
//        if (p <= 0 || p > 65535) {
//          cerr << "Invalid port: " << optarg << endl;
//          return false;
//        }
//        cfg.port = static_cast<uint16_t>(p);
//        break;
//      }
      case 'f':
        cfg.ssIrFileName = optarg;
        break;
      case 'd':
        cfg.dumpFileName = optarg;
        break;
//      case 'F':
//        cfg.fileStream = optarg;
//        break;
//      case 's': {
//        // -s or -s1 enables, -s0 disables
//        if (optarg != nullptr && string(optarg) == "0") {
//          cfg.doSingle = false;
//        } else {
//          cfg.doSingle = true;
//        }
//        break;
//      }
//      case 'S': {
//        if (optarg != nullptr && string(optarg) == "0") {
//          cfg.doStream = false;
//        } else {
//          cfg.doStream = true;
//        }
//        break;
//      }
//      case 't': {
//        int t = atoi(optarg);
//        if (t <= 0) {
//          cerr << "Invalid stream duration: " << optarg << endl;
//          return false;
//        }
//        cfg.streamSeconds = t;
//        break;
//      }
      case 'l':
        cfg.activeHigh = false;
        break;
      default:
        return false;
    }
  }

//  if (!cfg.doSingle && !cfg.doStream) {
//    cerr << "Error: at least one of single frame or stream must be enabled."
//        << endl;
//    return false;
//  }

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

int main(int argc, char **argv)
{
  if (!parseArgs(argc, argv, cfg)) {
    printHelp(argv[0]);
    return EXIT_FAILURE;
  }

  if (cfg.verbosity > 1) {
    //print raw hex, no parsing
    ifstream ssIrFile(cfg.ssIrFileName, ios::binary);
    if (!ssIrFile.is_open()) {
      cout << "file open error" << endl;
      return EXIT_FAILURE;
    }

    auto raw = std::vector<uint8_t>(std::istreambuf_iterator<char>(ssIrFile),
        std::istreambuf_iterator<char>());

    cout << lib::writeHex("ssIr input raw: ", raw);
  }

  ssIr::File streams;
  auto status = streams.parse(cfg.ssIrFileName);
  if (status != ssIr::Status::OK) {
    cout << "file parser error: " << (int) status << endl;
    return EXIT_FAILURE;
  }
  auto streamCount = streams.getStreamCount();
  if (streamCount == 0) {
    cout << "file is valid, but empty" << endl;
  } else {
    cout << "file is valid and contains " << streamCount << " IR streams"
        << endl;
  }

  for (int i = 0; i < streamCount; i++) {
    auto &s = streams.accessStream(i);

    cout << "----- IR stream " << i << " -----" << endl;
    cout << "Clock: " << s.getClock() << "kHz" << endl;

    auto &samples = s.accessStream();

    cout << "Sample count: " << samples.timings().size() * 2 << endl;

    if (cfg.verbosity > 0) {
      cout << "stream IR (hex): " << samples.convertHexString() << endl;
    }
    cout << "stream IR (tµs): " << samples.convertIntString() << endl;
    cout << samples.convertAsciiPlot(lib::getTerminalWidth(), cfg.activeHigh)
        << endl;

    if (!cfg.dumpFileName.empty()) {
      auto filename = lib::enumerateFilename(cfg.dumpFileName, i);
      //gnuplot
      auto str = samples.convertGnuplot(cfg.activeHigh);
      writeFile(filename, str);
    }
  }

  return EXIT_SUCCESS;
}
