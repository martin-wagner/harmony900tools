// SPDX-License-Identifier: LGPL-2.1-or-later

#include <boost/algorithm/string.hpp>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_utility.hpp>

#include "enum.h"

using namespace std;
using namespace magic_enum;

namespace document
{
namespace domain
{

template<typename T>
Enum<T>::Enum(const string &s) :
    src(s)
{
  static_assert(enum_contains<T>(T::Unknown), "T must have an Unknown member");

  auto v = enum_cast<T>(s, case_insensitive);
  if (v.has_value()) {
    value = v.value();
  } else {
    value = enum_cast<T>("Unknown", case_insensitive).value();
  }
}

template<typename T>
Enum<T>::Enum(T v) :
    value(v)
{
  src = enum_name(v);
}

template<typename T>
bool Enum<T>::isEnumValue(string s)
{
  boost::algorithm::to_lower(s);
  if (s == "unknown") {
    return false;
  }
  auto v = enum_cast<T>(s, case_insensitive);
  return v.has_value();
}

template<typename T>
string Enum<T>::getString(T v)
{
  return string(enum_name(v));
}

template<typename T>
QString Enum<T>::getQString(T v)
{
  return QString::fromStdString(getString(v));
}

template<typename T>
vector<string> Enum<T>::getStringList()
{
  vector<string> ret;

  enum_for_each<T>([&ret](auto val) {
    if (val != T::Unknown) {
      ret.push_back(getString(val));
    }
  });

  //we always want to have the current value available
  if (!isEnumValue(src)) {
    ret.push_back(src);
  }

  return ret;
}

template<typename T>
QStringList Enum<T>::getQStringList()
{
  QStringList ret;

  enum_for_each<T>([&ret](auto val) {
    if (val != T::Unknown) {
      ret.push_back(getQString(val));
    }
  });

  //we always want to have the current value available
  if (!isEnumValue(src)) {
    ret.push_back(QString::fromStdString(src));
  }

  return ret;
}

template<typename T>
string Enum<T>::getString() const
{
  return src;
}

template<typename T>
QString Enum<T>::getQString() const
{
  return QString::fromStdString(src);
}

template<typename T>
T Enum<T>::getValue() const
{
  return value;
}

}
}

