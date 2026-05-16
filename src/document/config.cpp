// SPDX-License-Identifier: LGPL-2.1-or-later

#include "config.h"

using namespace std;

namespace document
{

Config Config::create()
{
}

Config Config::create(const std::vector<uint8_t> &zip)
{
}

Config Config::create(const QString &path)
{
}

Config::~Config()
{
}

bool Config::isDirty()
{
}

QString Config::getPath()
{
}

bool Config::save()
{
}

bool Config::saveAs(const QString &path)
{
}

bool Config::dumpZip(std::vector<uint8_t> &zip)
{
}

void Config::activityAdded(int index)
{
}

Config::Config()
{
}

}
