#ifdef _WIN32
  #include <windows.h>
#endif
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <getopt.h>

#include "cli/lib/lib.h"
#include "file.h"
//#include "cli/lib/data.h"
#include "cli/lib/binary.h"
//#include "cli/lib/crc32.h"

using namespace std;

// --- config ---

struct Config
{
    int verbosity = 0;
    string irProtoFileName = "IrProto.bin";
    string dumpFileName = "protoIr.dat";
    string roundtripFileName = "";
    vector<string> addprotocols;
    double addStreamClock = 38000; //todo irProto::DEFAULT_CLOCK_HZ;
    bool activeHigh = true;
} cfg;

static void printHelp(const char *prog)
{
  cout << "Usage: " << prog << " [options]\n" << "\n" << "Options:\n"
      << "  -h, --help                  Show this help\n"
      << "  -v, --verbose               Verbose (repeat for -vv)\n"
      << "  -f, --file <file>           File to parse                (default: IrProto.bin)\n"
      << "  -d, --dump <file>           File to dump gnuplot output  (default: protoIr.dat, will be numbered)\n"
      << "  -l, --active-low            Active-low signal            (default: active-high)\n"
      << "  -r, --roundtrip <file>      Serialise parsed file back to <file> (roundtrip test)\n"
      << "  -s, --add-stream <string>   Append a stream in MP format (may be repeated)\n"
      << "                              Requires --roundtrip to write the result.\n"
      << "  -c, --clock <hz>            Carrier clock for added protocols (default: 38000)\n"
      << "\n" << "MP stream format:\n"
      << "  Alternating mark/pause pairs in microseconds, separated by semicolons.\n"
      << "  Each pair: MP<mark>:<pause>\n" << "\n"
      << "Example -- roundtrip test:\n" << "  " << prog
      << " -f IrProto.bin --roundtrip out.bin\n" << "\n"
      << "Example -- add a stream and write result:\n" << "  " << prog
      << " -f IrProto.bin --add-stream \"MP832:939; MP1778:896; MP882:1786; MP882:896;\" --roundtrip out.bin\n"
      << "\n" << "Example -- add two protocols with a custom clock:\n" << "  "
      << prog << " -f IrProto.bin \\\n"
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
        cfg.irProtoFileName = optarg;
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
        cfg.addprotocols.push_back(optarg);
        break;
      case 'c':
        cfg.addStreamClock = stod(optarg);
        break;
      default:
        return false;
    }
  }

  if (!cfg.addprotocols.empty() && cfg.roundtripFileName.empty()) {
    cout
        << "error: --add-stream requires --roundtrip <file> to write the result\n";
    return false;
  }

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

void printItems(const string &name, const vector<irProto::Item> &items,
    bool guessCode = false)
{
  std::vector<char> code;

  if (items.empty()) {
    cout << name << ": no data; ";
    return;
  }

  cout << name << " Items: " << items.size() << "; ";

  for (const auto &i : items) {
    if (i.first == true) {
      cout << "M" << i.second;
      code.push_back('M');
    } else {
      cout << "P" << i.second;
      code.push_back('P');
    }
    cout << ":";
  }

  cout << "; ";

  if (!guessCode) {
    return;
  }

  if (code.size() == 4) {
    if (code[0] == 'M' && code[1] == 'P' && code[2] == 'P' && code[3] == 'M') {
      cout << " -> Manchester A; ";
      return;
    }
    if (code[0] == 'P' && code[1] == 'M' && code[2] == 'M' && code[3] == 'P') {
      cout << " -> Manchester B; ";
      return;
    }
    if (code[0] == 'M' && code[1] == 'P' && code[2] == 'M' && code[3] == 'P') {
      cout << " -> Timing A; ";
      return;
    }
    if (code[0] == 'P' && code[1] == 'M' && code[2] == 'P' && code[3] == 'M') {
      cout << " -> Timing B; ";
      return;
    }
  }
}

int main(int argc, char **argv)
{
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  if (!parseArgs(argc, argv, cfg)) {
    printHelp(argv[0]);
    return EXIT_FAILURE;
  }

  if (cfg.verbosity > 1) {
    // print raw hex, no parsing
    ifstream irProtoFile(cfg.irProtoFileName, ios::binary);
    if (!irProtoFile.is_open()) {
      cout << "file open error" << endl;
      return EXIT_FAILURE;
    }

    auto raw = std::vector<uint8_t>(std::istreambuf_iterator<char>(irProtoFile),
        std::istreambuf_iterator<char>());

    cout << lib::writeHex("irProto raw: ", raw);

    auto crc = lib::parseHarmony32_file(raw[0], raw[1], raw[2], raw[3]);
    cout << "crc 1: " << hex << crc << dec << endl;
  }

  irProto::File protocols;
  auto status = protocols.parse(cfg.irProtoFileName);
  if (status != irProto::Status::OK) {
    cout << "file parser error: " << (int) status << endl;
    return EXIT_FAILURE;
  }
  auto protoCount = protocols.getProtocolCount();
  if (protoCount == 0) {
    cout << "file is valid, but empty" << endl;
  } else {
    cout << "file is valid and contains " << protoCount << " IR protocols"
        << endl;
  }

  for (int i = 0; i < protoCount; i++) {
    const auto &protocol = protocols.accessProtocol(i);

    cout << "----- IR protocol " << i << " -----" << endl;
    cout << "Clock: " << protocol.getClock() / 1000 << "kHz" << endl;

    auto sectionCount = protocol.getSectionCount();
    if (sectionCount == 0) {
      cout << "protocol is valid, but empty" << endl;
    } else {
      cout << "protocol is valid and contains " << sectionCount << " sections"
          << endl;
    }

    for (int j = 0; j < sectionCount; j++) {
      const auto &section = protocol.accessSection(j);

      cout << "--> section " << j << endl;
      cout << "Bits: " << section.getBitCount() << "; Mask: " << hex
          << section.getMask() << dec;
      auto timing = section.getTiming();
      if (timing == 0xffffffff) {
        cout << "; Interval: none; ";
      } else {
        cout << "; Interval: " << timing / 1000 << "ms; ";
      }
      auto ctrl0 = section.getCtrl0();
      switch (ctrl0) {
        case irProto::TimingSection::Ctrl0::IS_REPEAT_FRAME:
          cout << "Repeat Frame; ";
          break;
        case irProto::TimingSection::Ctrl0::IS_DATA_FRAME:
          cout << "Data Frame; ";
          break;
        default:
          cout << "Ctrl0: " << hex << (int) ctrl0 << dec << "; ";
          break;
      }

      //payload marker (??)
      switch (section.getCtrl1()) {
        case irProto::TimingSection::Ctrl1::NO_DATA:
          if (ctrl0 != irProto::TimingSection::Ctrl0::IS_REPEAT_FRAME) {
            cout << "Payload on repeat ???; ";
          }
          break;
        case irProto::TimingSection::Ctrl1::DATA_ONE_PAIR:
          //2 payload samples are required (see data coding)
          cout << "1 Payload Sample ???; ";
          break;
        case irProto::TimingSection::Ctrl1::DATA_TWO_PAIRS:
          if (ctrl0 == irProto::TimingSection::Ctrl0::IS_REPEAT_FRAME) {
            cout << "no Payload ???; ";
          }
          break;
        default:
          cout << "Ctrl1: " << hex << (int) section.getCtrl1() << dec << "; ";
          break;
      }

      const auto &sof = section.getSoF();
      const auto &data = section.getData();
      const auto &eof = section.getEoF();

      printItems("SoF", sof.accessStream());
      printItems("EoF", eof.accessStream());

      auto dataPrint = data.accessFalsePair();
      auto dataTrue = data.accessTruePair();
      dataPrint.insert(dataPrint.end(), dataTrue.begin(), dataTrue.end());

      printItems("D", dataPrint, true);
      cout << endl;
    }

//      if (!cfg.dumpFileName.empty()) {
//        auto filename = lib::enumerateFilename(cfg.dumpFileName, i);
//        // gnuplot
//        auto str = samples.convertGnuplot(cfg.activeHigh);
//        writeFile(filename, str);
//      }
  }

  // --- roundtrip / write back ---
  if (!cfg.roundtripFileName.empty()) {
    auto writeStatus = protocols.serialise(cfg.roundtripFileName);
    if (writeStatus != irProto::Status::OK) {
      cout << "error: roundtrip write failed: " << (int) writeStatus << endl;
      return EXIT_FAILURE;
    }
    cout << "wrote " << protocols.getProtocolCount() << " protocols(s) to "
        << cfg.roundtripFileName << endl;
  }

  return EXIT_SUCCESS;
}
