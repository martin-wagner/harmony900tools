// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
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

inline QString makeStringUnique(const QStringList &input, QString str,
    QString *msg = nullptr)
{
  auto baseStr = str;

  int suffix = 1;
  while (input.contains(str)) {
    str = baseStr + QString::number(suffix);
    suffix++;
  }
  if ((msg != nullptr) && (suffix > 1)) {
    *msg = QObject::tr("%1 already used. Names must be unique").arg(baseStr);
  }

  return str;
}

}

//outside namespace -> shorter

inline QString qstr(const std::string &str)
{
  return QString::fromStdString(str);
}

inline std::string strq(const QString &str)
{
  return str.toStdString();
}

