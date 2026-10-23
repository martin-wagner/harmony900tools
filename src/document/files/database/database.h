// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QFile>
#include <QFutureWatcher>
#include <QObject>
#include <QStringList>

#include <atomic>
#include <functional>

#include <nlohmann/json_fwd.hpp>

#include "document/data/enum.h"
#include "ui/logViewer.h"

namespace document
{
class Config;
namespace data
{
class CmdCatalogue;
namespace item
{
class Device;
}
}

namespace files
{

/**
 * Database files
 *
 * Imported / processed files, ready for further use.
 *
 * - catalogue -- json containing mnf/devicename/deviceid list for fast search
 * - raw -- one line per device id in json format, short form, mostly subset of "devices"
 * - devices -- one line per device id in json format, full dataset to import one device
 * - protocols -- json containing all protocols, corresponds to "devices"
 */
struct DatabasePaths
{
    //individual filenames. full path
    QString catalogue;
    QString raw;
    QString devices;
    QString protocols;

    bool valid() const { return (!catalogue.isEmpty() && !raw.isEmpty() && !devices.isEmpty() && !protocols.isEmpty()); };
    bool operator==(const DatabasePaths &) const = default;
};

inline static const QString catalogueFileName = "catalogue.json";
inline static const QString keyCatalogueVersion = "Version";
static constexpr uint32_t catalogueVersion = 1;
inline static const QString keyCatalogueLc = "LineCount";
inline static const QString keyCatalogueMnf = "Manufacturers";
inline static const QString keyCatalogueMnfName = "Name";
inline static const QString keyCatalogueDev = "Devices";
inline static const QString keyCatalogueDevName = "Name";
inline static const QString keyCatalogueDevType = "Type";
inline static const QString keyCatalogueDevId = "Id";
inline static const QString keyCatalogueDevLine = "Line";

//forward
class ImportDatabase;
class Database;

}
}
