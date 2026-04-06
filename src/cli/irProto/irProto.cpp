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
//#include "file.h"
#include "cli/lib/data.h"
#include "cli/lib/binary.h"
#include "cli/lib/crc32.h"

using namespace std;

// --- config ---

struct Config
{
    int verbosity = 0;
    string irProtoFileName = "IrProto.bin";
    string dumpFileName = "protoIr.dat";
    string roundtripFileName = "";
    vector<string> addStreams;
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
      << "  -c, --clock <hz>            Carrier clock for added streams (default: 38000)\n"
      << "\n" << "MP stream format:\n"
      << "  Alternating mark/pause pairs in microseconds, separated by semicolons.\n"
      << "  Each pair: MP<mark>:<pause>\n" << "\n"
      << "Example -- roundtrip test:\n" << "  " << prog
      << " -f IrProto.bin --roundtrip out.bin\n" << "\n"
      << "Example -- add a stream and write result:\n" << "  " << prog
      << " -f IrProto.bin --add-stream \"MP832:939; MP1778:896; MP882:1786; MP882:896;\" --roundtrip out.bin\n"
      << "\n" << "Example -- add two streams with a custom clock:\n" << "  "
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

void printItems(const string &name, int count, std::vector<uint16_t> items,
    bool guessCode = false)
{
  std::vector<char> code;

  if (count == 0) {
    cout << name << ": no data; ";
    return;
  }

  cout << name << " Items: " << count;
  if (count != items.size()) {
    cout << " (doesn't match: " << items.size() << ")";
  }
  cout << "; ";

  for (const auto &i : items) {
    if ((i & 0x8000) != 0) {
      cout << "M" << int(i & 0x7fff);
      code.push_back('M');
    } else {
      cout << "P" << i;
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

  ifstream irProtoFile(cfg.irProtoFileName, ios::binary);
  if (!irProtoFile.is_open()) {
    cout << "file open error" << endl;
    return EXIT_FAILURE;
  }

  auto raw = std::vector<uint8_t>(std::istreambuf_iterator<char>(irProtoFile),
      std::istreambuf_iterator<char>());

  if (cfg.verbosity > 1) {
    cout << lib::writeHex("irProto raw: ", raw);
  }

  //file consists of header{irProto structure}
  //decode header
  auto fileSizeExHeader = lib::parseHarmony16_file(raw[4], raw[5]);
  if (fileSizeExHeader != raw.size() - 8) {
    cout << "file sizes don't match (" << fileSizeExHeader << " -- "
        << raw.size() - 8 << endl;
  }
  cout << "file size: " << fileSizeExHeader << endl;

  auto crc32 = lib::calcCrc32( { raw.begin() + 8, raw.end() });
  cout << "crc/hash: " << hex << crc32 << dec << endl;

  auto crc32cmp = lib::parseHarmony32_file(raw[0], raw[1], raw[2], raw[3]);
  if (crc32 != crc32cmp) {
    cout << "crc mismatch, file: " << hex << crc32cmp << dec << endl;
    return EXIT_FAILURE;
  }

  auto unknownHeader = lib::parseHarmony16_file(raw[6], raw[7]);
  if (unknownHeader != 0) {
    cout << "unknown word not 0: " << hex << unknownHeader << dec << endl;
    return EXIT_FAILURE;
  }

  //easiest -> drop
  raw.erase(raw.begin(), raw.begin() + 8);

  //decode header -- done

  //5 byte header
  if (cfg.verbosity > 0) {
    //print header
    cout << lib::writeHex("irProto "
        "Header: ", vector<uint8_t>(raw.begin(), raw.begin() + 5));
  }
  //raw[6] unknown use, = 1
  if (cfg.verbosity > 0) {
    cout << "Byte 6: " << raw[5] << endl;
  }

  //command count
  auto arraySize = lib::parseHarmony16_file(raw[6], raw[7]);
  cout << "irProto Command count: " << arraySize << endl;

  //start into array
  //offset 5 for header
  vector<uint16_t> offsets;
  lib::parseHarmony16_file(
      { raw.begin() + 8, raw.begin() + 8 + 2 * arraySize }, offsets);

  if (cfg.verbosity > 1) {
    cout << lib::writeHex("irProto Command start offset (hex): ", offsets);
    cout << lib::writeData("irProto Command start offset (dec): ", offsets);
  }
  //easiest -> drop
  raw.erase(raw.begin(), raw.begin() + 5);

  //iterate over protocol objects
  for (int i = 0; i < offsets.size(); i++) {
    cout << "----- protocol object " << i << " -----" << endl;

    int start = offsets[i];
    int end = raw.size();
    if ((i + 1) < offsets.size()) {
      end = offsets[i + 1];
    }

    if (cfg.verbosity > 1) {
      cout
          << lib::writeHex("protocol object raw: ",
              vector<uint8_t> { raw.begin() + start, raw.begin() + end });
      cout << "size: " << end - start << endl;
    }

    uint32_t unknown = raw[start];
    if (unknown != 1) {
      cout << "start byte not 1: " << hex << unknown << dec << endl;
      return EXIT_FAILURE;
    }

    //carrier
    auto carrier = lib::parseHarmony16_file(raw[start + 1], raw[start + 2]);

    double carrier_ns = static_cast<double>(carrier) / 1000000000;
    double clock_khz = 1 / carrier_ns / 1000;

    cout << "clock: " << carrier_ns * 1000000 << "µs, " << clock_khz << "kHz"
        << endl;

    unknown = lib::parseHarmony16_file(raw[start + 3], raw[start + 4]);
    if (unknown != 0) {
      cout << "start word not 0: " << hex << unknown << dec << endl;
      return EXIT_FAILURE;
    }
    unknown = raw[start + 5]; //duty cycle (??)
    if (unknown != 50) {
      cout << "start byte not 50: " << hex << unknown << dec << endl;
      return EXIT_FAILURE;
    }

    uint8_t sectionCount = raw[start + 6];
    cout << "section count: " << (int) sectionCount << endl;

    //ende header

    vector<uint16_t> sectionOffsets;
    lib::parseHarmony16_file(
        { raw.begin() + start + 7, raw.begin() + start + 7 + 2 * sectionCount },
        sectionOffsets);

    if (cfg.verbosity > 1) {
      cout << lib::writeHex("section start offset (hex)", sectionOffsets);
      cout << lib::writeData("section start offset (dec)", sectionOffsets);
    }

    for (int j = 0; j < sectionCount; ++j) {
      cout << "--> section " << j << endl;

      int sectionStart = sectionOffsets[j];
      int sectionEnd = end;
      if ((j + 1) < sectionOffsets.size()) {
        sectionEnd = sectionOffsets[j + 1];
      }

      if (cfg.verbosity > 1) {
        cout
            << lib::writeHex("section object raw: ",
                vector<uint8_t> { raw.begin() + sectionStart, raw.begin()
                    + sectionEnd });
        cout << "size: " << sectionEnd - sectionStart << endl;
      }
      if ((sectionEnd - sectionStart) < 16) { //todo wie gross muss dieses ding mind. sein??
        cout << "section size error!!!!" << endl;
        continue;
      }

      auto bitCount = lib::parseHarmony16_file(raw[sectionStart + 0],
          raw[sectionStart + 1]);
      auto codeMask = lib::parseHarmony16_file(raw[sectionStart + 2],
          raw[sectionStart + 3]);
      auto repeatInterval = lib::parseHarmony32_file(raw[sectionStart + 4],
          raw[sectionStart + 5], raw[sectionStart + 6], raw[sectionStart + 7]);
      auto body0 = raw[sectionStart + 8];
      auto body1 = raw[sectionStart + 9];
      //order is weird
      auto ptr1Offset = lib::parseHarmony16_file(raw[sectionStart + 12],
          raw[sectionStart + 13]);
      auto ptr2Offset = lib::parseHarmony16_file(raw[sectionStart + 14],
          raw[sectionStart + 15]);
      auto ptr3Offset = lib::parseHarmony16_file(raw[sectionStart + 10],
          raw[sectionStart + 11]);

      if (cfg.verbosity > 0) {
        cout << "section bit count: " << bitCount << " : 0x" << hex << bitCount
            << dec << endl;
        cout << "codemask: " << codeMask << " : 0x" << hex << codeMask << dec
            << endl;
        //interval from start to next start (? verify)
        cout << "interval: " << repeatInterval << " : 0x" << hex
            << repeatInterval << dec << endl;
        cout << "body 0/1: " << hex << (int) body0 << " : " << (int) body1
            << dec << endl;
        cout << "offset ptr1 (start / bit0): " << ptr1Offset << " : " << hex
            << ptr1Offset << dec << endl;
        cout << "offset ptr2 (stop / bit1): " << ptr2Offset << " : " << hex
            << ptr2Offset << dec << endl;
        cout << "offset ptr3 (data / hdr): " << ptr3Offset << " : " << hex
            << ptr3Offset << dec << endl;
      }

      int ptr1ItemCount = 0;
      vector<uint16_t> ptr1timings;
      int ptr2ItemCount = 0;
      vector<uint16_t> ptr2timings;
      int ptr3ItemCount = 0;
      vector<uint16_t> ptr3timings;

      //start sequence (?)
      if (ptr1Offset != 0) {
        ptr1ItemCount = raw[ptr1Offset];

        auto start = raw.begin() + ptr1Offset + 1;
        lib::parseHarmony16_file( { start, start + 2 * ptr1ItemCount },
            ptr1timings);

        if (cfg.verbosity > 1) {
          cout << "ptr1 item count: " << (int) ptr1ItemCount << endl;
          cout << lib::writeHex("in-protocol ptr1 data (hex)", ptr1timings);
          cout << lib::writeData("in-protocol ptr1 data (dec)", ptr1timings);
        }
      }

      //stop sequence (?)
      if (ptr2Offset != 0) {
        ptr2ItemCount = raw[ptr2Offset];

        auto start = raw.begin() + ptr2Offset + 1;
        lib::parseHarmony16_file( { start, start + 2 * ptr2ItemCount },
            ptr2timings);

        if (cfg.verbosity > 1) {
          cout << "ptr2 item count: " << (int) ptr2ItemCount << endl;
          cout << lib::writeHex("in-protocol ptr2 data (hex)", ptr2timings);
          cout << lib::writeData("in-protocol ptr2 data (dec)", ptr2timings);
        }
      }

      //data coding 0 / 1 (todo order correct?), should always be 4 timing values (two for coding "0" and two for coding "1")
      //coding seems to either use manchester or variable-length pairs
      if (ptr3Offset != 0) {
        lib::parseHarmony16_file(
            { raw.begin() + ptr3Offset, raw.begin() + sectionEnd },
            ptr3timings);
        ptr3ItemCount = ptr3timings.size();
        if (cfg.verbosity > 1) {
          cout << "ptr1 item count: " << (int) ptr1ItemCount
              << " (should always be 4)" << endl;
          cout << lib::writeHex("in-protocol ptr3 data (hex)", ptr3timings);
          cout << lib::writeData("in-protocol ptr3 data (dec)", ptr3timings);
        }
      }

      cout << "Bits: " << bitCount << "; Mask: " << hex << codeMask << dec;
      if (repeatInterval == 0xffffffff) {
        cout << "; Interval: none; ";
      } else {
        cout << "; Interval: " << repeatInterval / 1000 << "ms; ";
      }
      //work marker (??)
      switch (body0) {
        case 0:
          cout << "Repeat Frame; ";
          break;
        case 2:
          cout << "Data Frame; ";
          break;
        default:
          cout << "Body 0: " << hex << (int) body0 << dec << "; ";
          break;
      }

      //payload marker (??)
      switch (body1) {
        case 0:
          if (body0 != 0) {
            cout << "Payload on repeat ???; ";
          }
          break;
        case 1:
          //2 payload samples are required (see data coding)
          cout << "1 Payload Sample ???; ";
          break;
        case 2:
          if (body0 == 0) {
            cout << "no Payload ???; ";
          }
          break;
        default:
          cout << "Body 1: " << hex << (int) body0 << dec << "; ";
          break;
      }
      printItems("SoF", ptr1ItemCount, ptr1timings);
      printItems("EoF", ptr2ItemCount, ptr2timings);
      printItems("D", 2 * body1, ptr3timings, true);

      cout << endl;

    }
    cout << "--------" << endl;

  }

  return EXIT_SUCCESS;
}
