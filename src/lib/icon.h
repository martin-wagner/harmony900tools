/*
 * icon.h
 *
 *  Created on: Apr 26, 2026
 *      Author: martin
 */

#pragma once

#include <QIcon>

namespace lib
{

inline QIcon getIcon(const QString &icon, const QString &fromTheme = "")
{
  return QIcon::fromTheme(fromTheme, QIcon(icon));
  //for testing
  //return QIcon(icon);
}

}
