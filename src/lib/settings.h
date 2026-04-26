/*
 * settings.h
 *
 *  Created on: Apr 26, 2026
 *      Author: martin
 */

#pragma once

#include <QSettings>
#include <QCoreApplication>

namespace lib
{

inline QSettings getQSettings()
{
  return QSettings(QCoreApplication::organizationName(),
      QCoreApplication::applicationName());
}

}
