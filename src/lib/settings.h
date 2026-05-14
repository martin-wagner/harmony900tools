// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QSettings>
#include <QCoreApplication>

namespace lib
{

inline QSettings getQSettings()
{
#ifdef _WIN32
    return QSettings(QSettings::IniFormat,
                     QSettings::UserScope,
                     QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
#else
    return QSettings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
#endif
}

inline void resetQSettings()
{
  auto settings = getQSettings();
  settings.clear();
  settings.sync();
}

}
