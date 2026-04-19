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
#include "code.h"
#include "cli/lib/binary.h"

using namespace std;

// --- config ---

struct Config
{
    int verbosity = 0;
    string irProtoFileName = "IrProto.bin";
    string dumpFileName = "cmd.dat";
    string roundtripFileName = "";
    string commandHex = "";
    bool activeHigh = true;
} cfg;

static void printHelp(const char *prog)
{
  cout << "Usage: " << prog << " [options]\n" << "\n" << "Options:\n"
      << "  -h, --help                  Show this help\n"
      << "  -v, --verbose               Verbose (repeat for -vv)\n"
      << "  -f, --file <file>           File to parse                (default: IrProto.bin)\n"
      << "  -c, --command <hex>         Command string, corresponding to IrProto.bin\n"
      << "  -d, --dump <file>           File to dump gnuplot output of command  (default: cmd.dat)\n"
      << "  -l, --active-low            Active-low signal            (default: active-high)\n"
      << "  -r, --roundtrip <file>      Serialise parsed file back to <file> (roundtrip test)\n"
      << "Example -- roundtrip test:\n" << "  " << prog
      << " -f IrProto.bin --roundtrip out.bin\n" << "\n"
      << "Example -- command to IR:\n" << "  " << prog
      << " -f IrProto.bin -c \"0x0000F401010100E1A2E817010100\" -d\n" << "\n";
}

static bool parseArgs(int argc, char **argv, Config &cfg)
{
  // @formatter:off
  static struct option long_options[] = {
      { "help",       no_argument,       nullptr, 'h' },
      { "verbose",    no_argument,       nullptr, 'v' },
      { "file",       required_argument, nullptr, 'f' },
      { "dump",       required_argument, nullptr, 'd' },
      { "active-low", no_argument,       nullptr, 'l' },
      { "roundtrip",  required_argument, nullptr, 'r' },
      { "command",    required_argument, nullptr, 'c' },
      { nullptr, 0, nullptr, 0 }
  };
// @formatter:on

  int opt;
  int optionIndex = 0;
  while ((opt = getopt_long(argc, argv, "hvf:d:lr:c:", long_options,
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
      case 'c':
        cfg.commandHex = optarg;
        break;
      default:
        return false;
    }
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
  }

  if (!cfg.commandHex.empty()) {
    lib::TimingStream samples;
    irProto::Code code;

    auto status = code.parse(cfg.commandHex);
    if (status != irProto::Status::OK) {
      cout << "command parser error: " << (int) status << endl;
      return EXIT_FAILURE;
    }
    auto data = code.getData();
    auto index = code.getIndex();
    status = protocols.serialiseIrStream(samples, index, data);
    if (status != irProto::Status::OK) {
      cout << "command encoding error: " << (int) status << endl;
      return EXIT_FAILURE;
    }

    cout << "----- IR stream (Protocol " << index << ") -----" << endl;
    cout << "Clock: " << protocols.accessProtocol(index).getClock() / 1000
        << "kHz" << endl;

    cout << "Sample count: " << samples.timings().size() * 2 << endl;

    if (cfg.verbosity > 0) {
      cout << "stream IR (hex): " << samples.convertHexString() << endl;
    }
    cout << "stream IR (tµs): " << samples.convertIntString() << endl;
    cout << samples.convertAsciiPlot(lib::getTerminalWidth(), cfg.activeHigh)
        << endl;

    if (!cfg.dumpFileName.empty()) {
      // gnuplot
      auto str = samples.convertGnuplot(cfg.activeHigh);
      writeFile(cfg.dumpFileName, str);
    }
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
