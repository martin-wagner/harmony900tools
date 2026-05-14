// SPDX-License-Identifier: LGPL-2.1-or-later

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
