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

int main(int argc, char **argv)
{
  if (!parseArgs(argc, argv, cfg)) {
    printHelp(argv[0]);
    return EXIT_FAILURE;
  }

  //todo
  cfg.verbosity = 2;

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
    cout << "add 5 bytes to find start in hexdump" << endl;
  }
  //easiest -> drop
  raw.erase(raw.begin(), raw.begin() + 5);

  //iterate over protocol objects
  for (int i = 0; i < offsets.size(); i++) {
    cout << "----- protocol object " << i << "-----" << endl;

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

    vector<uint16_t> protOffsets;
    lib::parseHarmony16_file(
        { raw.begin() + start + 7, raw.begin() + start + 7 + 2 * sectionCount },
        protOffsets);

    if (cfg.verbosity > 1) {
      cout << lib::writeHex("in-protocol start offset (hex)", protOffsets);
      cout << lib::writeData("in-protocol start offset (dec)", protOffsets);
    }


    for (int j = 0; j < sectionCount; ++j) {
      cout << "--> protocol object item " << j << endl;

      int protStart = protOffsets[j];
      int protEnd = end;
      if ((j + 1) < protOffsets.size()) {
        protEnd = protOffsets[j + 1];
      }

      if (cfg.verbosity > 1) {
        cout
            << lib::writeHex("protocol object item raw: ",
                vector<uint8_t> { raw.begin() + protStart, raw.begin() + protEnd });
        cout << "size: " << protEnd - protStart << endl;
      }
      if ((protEnd - protStart) < 16) { //todo wie gross muss dieses ding mind. sein??
        cout << "size error!!!!" << endl;
        continue;
      }


      auto bit_count = lib::parseHarmony16_file(raw[protStart + 0], raw[protStart + 1]);
      cout << "bitcount: " << bit_count << " : 0x" << hex << bit_count << dec  << endl;

      auto code_mask = lib::parseHarmony16_file(raw[protStart + 2], raw[protStart + 3]);
      cout << "codemask: " << code_mask <<  " : 0x" << hex << code_mask << dec << endl;

      auto repeat_interval = lib::parseHarmony32_file(raw[protStart + 4], raw[protStart + 5], raw[protStart + 6], raw[protStart + 7]);
      cout << "repeat: " << repeat_interval <<  " : 0x" << hex << repeat_interval << dec << endl;

      auto body0 = raw[protStart + 8];
      auto body1 = raw[protStart + 9];
      cout << "body: " << hex << (int)body0 << " : " << (int)body1 << dec << endl;

      //pointer to last segment
      auto ptr3Offset = lib::parseHarmony16_file(raw[protStart + 10], raw[protStart + 11]);
      cout << "offset ptr3 (data / hdr): " << ptr3Offset <<  " : " << hex << ptr3Offset << dec << endl;
      //pointer to first segment
      auto ptr1Offset = lib::parseHarmony16_file(raw[protStart + 12], raw[protStart + 13]);
      cout << "offset ptr1 (start / bit0): " << ptr1Offset <<  " : " << hex << ptr1Offset << dec << endl;
      //pointer to second segment (??optional, 0 if not used)
      auto ptr2Offset = lib::parseHarmony16_file(raw[protStart + 14], raw[protStart + 15]);
      cout << "offset ptr2 (stop / bit1): " << ptr2Offset <<  " : " << hex << ptr2Offset << dec << endl;


      if (ptr1Offset != 0) {
        auto listEnd = ptr2Offset;
        if (listEnd == 0) {
          listEnd = ptr3Offset;
        }
        if (listEnd == 0) {
          listEnd = protEnd;
        }

        auto ptr1header = raw[ptr1Offset];
        cout << "ptr1 counter: " << (int)ptr1header << endl;

        vector<uint16_t> timings;
        lib::parseHarmony16_file( { raw.begin() + ptr1Offset + 1, raw.begin() + listEnd }, timings);
        cout << lib::writeHex("in-protocol ptr1 data (hex)", timings);
        cout << lib::writeData("in-protocol ptr1 data (dec)", timings);
        for (auto &t : timings) {
          t = t & 0x7fff;
        }
        cout << lib::writeData("in-protocol ptr1 data, masked (dec)", timings);
      }


      if (ptr2Offset != 0) {
        auto listEnd = ptr3Offset;
        if (listEnd == 0) {
          listEnd = protEnd;
        }

        auto ptr2header = raw[ptr2Offset];
        cout << "ptr2 counter: " << (int)ptr2header << endl;

        vector<uint16_t> timings;
        lib::parseHarmony16_file( { raw.begin() + ptr2Offset + 1, raw.begin() + listEnd }, timings);
        cout << lib::writeHex("in-protocol ptr2 data (hex)", timings);
        cout << lib::writeData("in-protocol ptr2 data (dec)", timings);
        for (auto &t : timings) {
          t = t & 0x7fff;
        }
        cout << lib::writeData("in-protocol ptr1 data, masked (dec)", timings);
      }

      if (ptr3Offset != 0) {
        auto listEnd = protEnd;

        vector<uint16_t> timings;
        lib::parseHarmony16_file( { raw.begin() + ptr3Offset, raw.begin() + listEnd }, timings);
        cout << lib::writeHex("in-protocol ptr3 data (hex)", timings);
        cout << lib::writeData("in-protocol ptr3 data (dec)", timings);
        for (auto &t : timings) {
          t = t & 0x7fff;
        }
        cout << "data 0-1 / 1-0" << endl;
        cout << lib::writeData("in-protocol ptr1 data, masked (dec)", timings);
      }


    }

  }

  return EXIT_SUCCESS;
}
