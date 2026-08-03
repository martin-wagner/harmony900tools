// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStringList>
#include <QString>
#include <string>
#include <vector>

namespace lib
{

inline QStringList toQStringList(const std::vector<std::string> &strings)
{
  QStringList result;
  result.reserve(static_cast<qsizetype>(strings.size()));

  for (const std::string &str : strings) {
    result.append(QString::fromStdString(str));
  }

  return result;
}

inline std::vector<std::string> toStdVector(const QStringList &strings)
{
  std::vector<std::string> result;
  result.reserve(static_cast<size_t>(strings.size()));

  for (const QString &str : strings) {
    result.emplace_back(str.toStdString());
  }

  return result;
}

}
