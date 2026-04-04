#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <getopt.h>

#include "cli/lib/lib.h"
#include "file.h"

using namespace std;

// --- config ---

struct Config
{
    int verbosity = 0;
    string ssIrFileName = "SsIr.bin";
    string dumpFileName = "streamIr.dat";
    string roundtripFileName = "";
    vector<string> addStreams;
    double addStreamClock = ssIr::DEFAULT_CLOCK_HZ;
    bool activeHigh = true;
} cfg;

static void printHelp(const char *prog)
{
  cout << "Usage: " << prog << " [options]\n" << "\n" << "Options:\n"
      << "  -h, --help                  Show this help\n"
      << "  -v, --verbose               Verbose (repeat for -vv)\n"
      << "  -f, --file <file>           File to parse                (default: SsIr.bin)\n"
      << "  -d, --dump <file>           File to dump gnuplot output  (default: streamIr.dat, will be numbered)\n"
      << "  -l, --active-low            Active-low signal            (default: active-high)\n"
      << "  -r, --roundtrip <file>      Serialise parsed file back to <file> (roundtrip test)\n"
      << "  -s, --add-stream <string>   Append a stream in MP format (may be repeated)\n"
      << "                              Requires --roundtrip to write the result.\n"
      << "  -c, --clock <hz>            Carrier clock for added streams (default: 38000)\n"
      << "\n" << "MP stream format:\n"
      << "  Alternating mark/pause pairs in microseconds, separated by semicolons.\n"
      << "  Each pair: MP<mark>:<pause>\n" << "\n"
      << "Example -- roundtrip test:\n" << "  " << prog
      << " -f SsIr.bin --roundtrip out.bin\n" << "\n"
      << "Example -- add a stream and write result:\n" << "  " << prog
      << " -f SsIr.bin --add-stream \"MP832:939; MP1778:896; MP882:1786; MP882:896;\" --roundtrip out.bin\n"
      << "\n" << "Example -- add two streams with a custom clock:\n" << "  "
      << prog << " -f SsIr.bin \\\n"
      << "    --add-stream \"MP832:939; MP1778:896;\" \\\n"
      << "    --add-stream \"MP500:500; MP1000:500;\" \\\n"
      << "    --clock 38000 --roundtrip out.bin\n" << "\n";
}

static bool parseArgs(int argc, char **argv, Config &cfg)
{
  static struct option long_options[] = { { "help", no_argument, nullptr, 'h' },
      { "verbose", no_argument, nullptr, 'v' }, { "file", required_argument,
          nullptr, 'f' }, { "dump", required_argument, nullptr, 'd' }, {
          "active-low", no_argument, nullptr, 'l' }, { "roundtrip",
      required_argument, nullptr, 'r' }, { "add-stream", required_argument,
          nullptr, 's' }, { "clock", required_argument, nullptr, 'c' }, {
          nullptr, 0, nullptr, 0 } };

  int opt;
  int optionIndex = 0;
  while ((opt = getopt_long(argc, argv, "hvf:d:lr:s:c:", long_options,
      &optionIndex)) != -1) {
    switch (opt) {
      case 'h':
        return false;
      case 'v':
        cfg.verbosity++;
        break;
      case 'f':
        cfg.ssIrFileName = optarg;
        break;
      case 'd':
        cfg.dumpFileName = optarg;
        break;
      case 'l':
        cfg.activeHigh = false;
        break;
      case 'r':
        cfg.roundtripFileName = optarg;
        break;
      case 's':
        cfg.addStreams.push_back(optarg);
        break;
      case 'c':
        cfg.addStreamClock = stod(optarg);
        break;
      default:
        return false;
    }
  }

  if (!cfg.addStreams.empty() && cfg.roundtripFileName.empty()) {
    cout
        << "error: --add-stream requires --roundtrip <file> to write the result\n";
    return false;
  }

  return true;
}

/**
 * Parse a stream string in MP format: "MP832:939; MP1778:896; MP882:1786;"
 * Returns alternating mark, pause values in microseconds.
 */
static bool parseStreamString(const string &input, vector<uint16_t> &out)
{
  out.clear();
  istringstream ss(input);
  string token;

  while (getline(ss, token, ';')) {
    // trim leading whitespace
    size_t start = token.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
      continue; // empty token after trailing semicolon
    }
    token = token.substr(start);

    if (token.empty()) {
      continue;
    }

    // expect "MP<mark>:<pause>"
    if (token.size() < 4 || token[0] != 'M' || token[1] != 'P') {
      cout << "error: invalid stream token (expected MPmark:pause): \"" << token
          << "\"\n";
      return false;
    }

    token = token.substr(2); // strip "MP"
    size_t colonPos = token.find(':');
    if (colonPos == string::npos) {
      cout << "error: missing ':' in token: \"" << token << "\"\n";
      return false;
    }

    string markStr = token.substr(0, colonPos);
    string pauseStr = token.substr(colonPos + 1);

    try {
      uint16_t mark = static_cast<uint16_t>(stoul(markStr));
      uint16_t pause = static_cast<uint16_t>(stoul(pauseStr));
      out.push_back(mark);
      out.push_back(pause);
    } catch (const exception &e) {
      cout << "error: could not parse numbers in token \"" << token << "\": "
          << e.what() << "\n";
      return false;
    }
  }

  if (out.empty()) {
    cout << "error: stream string parsed to zero samples\n";
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

int main(int argc, char **argv)
{
  if (!parseArgs(argc, argv, cfg)) {
    printHelp(argv[0]);
    return EXIT_FAILURE;
  }

  if (cfg.verbosity > 1) {
    // print raw hex, no parsing
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
      // gnuplot
      auto str = samples.convertGnuplot(cfg.activeHigh);
      writeFile(filename, str);
    }
  }

  // --- add streams from CLI ---
  for (const auto &streamStr : cfg.addStreams) {
    vector<uint16_t> data;
    if (!parseStreamString(streamStr, data)) {
      return EXIT_FAILURE;
    }

    int addedIndex = -1;
    auto addStatus = streams.appendStream(data, cfg.addStreamClock, addedIndex);
    if (addStatus != ssIr::Status::OK) {
      cout << "error: appendStream failed: " << (int) addStatus << endl;
      return EXIT_FAILURE;
    }
    cout << "added stream at index " << addedIndex << " (total now: "
        << streams.getStreamCount() << ")" << " | samples: " << data.size()
        << " | clock: " << cfg.addStreamClock << " Hz" << endl;
  }

  // --- roundtrip / write back ---
  if (!cfg.roundtripFileName.empty()) {
    auto writeStatus = streams.serialise(cfg.roundtripFileName);
    if (writeStatus != ssIr::Status::OK) {
      cout << "error: roundtrip write failed: " << (int) writeStatus << endl;
      return EXIT_FAILURE;
    }
    cout << "wrote " << streams.getStreamCount() << " stream(s) to "
        << cfg.roundtripFileName << endl;
  }

  return EXIT_SUCCESS;
}
