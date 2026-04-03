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
#include "cli/lib/binary.h"
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
//    string fileStream = "streaming_ir.dat";
//    bool doSingle = true;
//    bool doStream = false;
//    int streamSeconds = 5;
//    bool activeHigh = true;
} cfg;

static void printHelp(const char *prog)
{
  cout << "Usage: " << prog << " [options]\n" << "\n" << "Options:\n"
      << "  -h             Show this help\n"
      << "  -v             Verbose (repeat for -vv)\n"
//      << "  -i <ip>        IP address         (default: 169.254.1.2)\n"
//      << "  -p <port>      Port               (default: 3074)\n"
      << "  -f <file>      File to parse        (default: SsIr.bin)\n"
//      << "  -F <file>      Stream file        (default: streaming_ir.dat)\n"
//      << "  -s[0|1]        Poll single frame  (default: on,  -s0 disables)\n"
//      << "  -S[0|1]        Poll stream        (default: off, -S1 enables)\n"
//      << "  -t <seconds>   Stream duration    (default: 5)\n"
//      << "  -l             Active-low signal  (default: active-high)\n" << "\n"
//      << "At least one of -s/-S (or their defaults) must be active.\n"
      ;
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
//      case 'l':
//        cfg.activeHigh = false;
//        break;
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
//
//bool writeFile(const string &filename, const string &data)
//{
//  ofstream file(filename);
//
//  if (!file.is_open()) {
//    return false;
//  }
//
//  file << data;
//
//  file.close();
//
//  return true;
//}

int main(int argc, char **argv)
{
  if (!parseArgs(argc, argv, cfg)) {
    printHelp(argv[0]);
    return EXIT_FAILURE;
  }

  ifstream ssIrFile(cfg.ssIrFileName, ios::binary);
  if (!ssIrFile.is_open()) {
    cout << "file open error" << endl;
    return EXIT_FAILURE;
  }

  auto raw = std::vector<uint8_t>(std::istreambuf_iterator<char>(ssIrFile),
      std::istreambuf_iterator<char>());

  if (cfg.verbosity > 1) {
    cout << lib::writeHex("ssIr raw: ", raw);
  }

  //5 byte header
  if (cfg.verbosity > 0) {
    //print header
    cout << lib::writeHex("ssIr "
        "Header: ", vector<uint8_t>(raw.begin(), raw.begin() + 5));
  }
  //raw[6] unknown use, = 1
  if (cfg.verbosity > 0) {
    cout << "Byte 6: " << raw[5] << endl;
  }

  //command count
  auto arraySize = lib::parseHarmony16_file(raw[6], raw[7]);
  cout << "ssIr Command count: " << arraySize << endl;

  //start into array
  //offset 5 for header
  vector<uint16_t> offsets;
  lib::parseHarmony16_file(
      { raw.begin() + 8, raw.begin() + 8 + 2 * arraySize }, offsets);

  if (cfg.verbosity > 1) {
    cout << lib::writeHex("ssIr Command start offset (hex): ", offsets);
    cout << lib::writeData("ssIr Command start offset (dec): ", offsets);
    cout << "add 5 bytes to find start in hexdump" << endl;
  }

  for (int i = 0; i < offsets.size(); i++) {
    cout << "----- raw command " << i << "-----" << endl;

    int start = offsets[i] + 5;
    int end = raw.size();
    if ((i + 1) < offsets.size()) {
      end = offsets[i + 1] + 5;
    }

    //payload data
    vector<uint16_t> payload;
    auto ret = lib::parseHarmony16_file(
        { raw.begin() + start, raw.begin() + end }, payload);
    if (ret != true) {
      cout << "ssIr file size error" << endl;
      return EXIT_FAILURE;
    }

    if (cfg.verbosity > 1) {
      cout << lib::writeHex("payload (hex)", payload);
      cout << lib::writeData("payload (hex)", payload);
    }

    double carrier_ns = static_cast<double>(payload[0]) / 1000000000;
    double clock_khz = 1 / carrier_ns / 1000;

    cout << "clock: " << carrier_ns * 1000000 << "µs, " << clock_khz << "kHz"
        << endl;

    if (payload[1] != 0) {
      cout << "??? should be 0000: " << payload[1] << endl;
      return EXIT_FAILURE;
    }

    auto sampleCount = payload[2];

    cout << "sample count: " << sampleCount << endl;

    for (int j = 3; j < payload.size(); j++) {
      const uint16_t sample = payload[j];

      if ((sample & 0x8000) != 0) {
        cout << "mark: " << (sample & 0x7fff) << endl;
      } else {
        cout << "space: " << sample << endl;

      }
    }
  }

  return EXIT_SUCCESS;
}
