// SPDX-License-Identifier: LGPL-2.1-or-later

#ifdef _WIN32
  #include <windows.h>
#endif
#include <iostream>
#include <getopt.h>

#include <QString>
#include <QStringList>

#include "lib/qtHelpers.h"
#include "document/files/database.h"

using namespace std;

using namespace document::files;

// --- config ---

struct Config
{
    int verbosity = 0;
    string catalogueFileName = "catalogue.json";
    string manufacturerSearch = "";
    string deviceSearch = "";
} cfg;

static void printHelp(const char *prog)
{
  cout << "Usage: " << prog << " [options]\n" << "\n" << "Options:\n"
      << "  -h, --help                  Show this help\n"
      << "  -v, --verbose               Verbose (repeat for -vv)\n"
      << "  -c, --catalogue <file>      Catalogue file             (default: catalogue.json)\n"
      << "  -m, --manufacturer <substr> Search manufacturers by substring\n"
      << "  -d, --device <substr>       Search devices by substring\n"
      << "Notes:\n"
      << "  If both -m and -d are given, devices are only searched if the\n"
      << "  manufacturer search yields exactly one match.\n"
      << "Example -- search manufacturers:\n" << "  " << prog << " -m sony\n"
      << "\n" << "Example -- search devices of a single manufacturer:\n" << "  "
      << prog << " -m sony -d tv\n" << "\n";
}

static bool parseArgs(int argc, char **argv, Config &cfg)
{
  // @formatter:off
  static struct option long_options[] = {
      { "help",         no_argument,       nullptr, 'h' },
      { "verbose",      no_argument,       nullptr, 'v' },
      { "catalogue",    required_argument, nullptr, 'c' },
      { "manufacturer", required_argument, nullptr, 'm' },
      { "device",       required_argument, nullptr, 'd' },
      { nullptr, 0, nullptr, 0 }
  };
// @formatter:on

  int opt;
  int optionIndex = 0;
  while ((opt = getopt_long(argc, argv, "hvc:m:d:", long_options, &optionIndex))
      != -1) {
    switch (opt) {
      case 'h':
        return false;
      case 'v':
        cfg.verbosity++;
        break;
      case 'c':
        cfg.catalogueFileName = optarg;
        break;
      case 'm':
        cfg.manufacturerSearch = optarg;
        break;
      case 'd':
        cfg.deviceSearch = optarg;
        break;
      default:
        return false;
    }
  }

  return true;
}

static void printStringList(const QStringList &list)
{
  cout << list.join("; ").toStdString() << endl;
}

static void printDeviceInfo(const Database::Device &device)
{
  cout << "Name: " << strq(device.name) << "; ";
  cout << "Type: " << strq(device.type.getQString()) << "; ";
  cout << "ID: " << device.id << "; ";
  cout << "Line: " << device.line << endl;
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

  DatabasePaths paths;
  paths.catalogue = qstr(cfg.catalogueFileName);
  paths.raw = "catalog_raw.jsonl"; // placeholder: not yet exposed via CLI options
  paths.devices = "device_features_raw.jsonl"; // placeholder: not yet exposed via CLI options
  paths.protocols = "AllProtocols.json"; // placeholder: not yet exposed via CLI options

  cout << "Parsing files..." << endl;
  Database db;
  if (!db.setDatabase(paths)) {
    cout << "database load error" << endl;
    return EXIT_FAILURE;
  }
  cout << "Done." << endl;

  if (cfg.verbosity > 0) {
    cout << "device count: " << db.getDeviceCount() << endl;
  }

  auto hasManufacturerSearch = !cfg.manufacturerSearch.empty();
  auto hasDeviceSearch = !cfg.deviceSearch.empty();

  if ((hasManufacturerSearch == true) && (hasDeviceSearch == true)) {
    auto manufacturerMatches = db.findManufacturers(
        qstr(cfg.manufacturerSearch));

    QString manufacturer;
    auto haveUniqueManufacturer = false;

    if (manufacturerMatches.size() == 1) {
      manufacturer = manufacturerMatches.first();
      haveUniqueManufacturer = true;
    } else if (manufacturerMatches.size() > 1) {
      for (const auto &candidate : manufacturerMatches) {
        if (QString::compare(candidate, qstr(cfg.manufacturerSearch),
            Qt::CaseInsensitive) == 0) {
          manufacturer = candidate;
          haveUniqueManufacturer = true;
          break;
        }
      }
    }

    if (haveUniqueManufacturer == true) {
      auto deviceMatches = db.findDevices(manufacturer, qstr(cfg.deviceSearch));
      cout << "Manufacturer: " << strq(manufacturer) << " --> Devices: ";
      printStringList(deviceMatches);

      if (deviceMatches.size() == 1) {
        auto device = db.getDeviceInfo(manufacturer, deviceMatches.first());
        printDeviceInfo(device);
      }
    } else {
      printStringList(manufacturerMatches);
      if (manufacturerMatches.isEmpty()) {
        cout << "# no devices shown: manufacturer search matched nothing"
            << endl;
      } else {
        cout << "# no devices shown: manufacturer search matched "
            << manufacturerMatches.size() << " manufacturers, not exactly 1"
            << endl;
      }
    }
  } else if (hasManufacturerSearch == true) {
    cout << "Manufacturers: ";
    printStringList(db.findManufacturers(qstr(cfg.manufacturerSearch)));
  } else if (hasDeviceSearch == true) {
    cout << "Devices: ";
    printStringList(db.findDevices(qstr(cfg.deviceSearch)));
  } else {
    printHelp(argv[0]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
