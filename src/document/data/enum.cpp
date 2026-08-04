// SPDX-License-Identifier: LGPL-2.1-or-later

#include <boost/algorithm/string.hpp>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_utility.hpp>

#include "enum.h"

using namespace std;
using namespace magic_enum;

namespace document
{
namespace data
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
string Enum<T>::toString(T v)
{
  return string(enum_name(v));
}

template<typename T>
QString Enum<T>::toQString(T v)
{
  return QString::fromStdString(toString(v));
}

template<typename T>
inline std::vector<std::string> document::data::Enum<T>::toStringList()
{
  vector<string> ret;

  enum_for_each<T>([&ret](auto val) {
    if (val != T::Unknown) {
      ret.push_back(toString(val));
    }
  });

  return ret;
}

template<typename T>
inline QStringList document::data::Enum<T>::toQStringList()
{
  QStringList ret;

  enum_for_each<T>([&ret](auto val) {
    if (val != T::Unknown) {
      ret.push_back(toQString(val));
    }
  });

  return ret;
}

template<typename T>
vector<string> Enum<T>::getStringList() const
{
  auto ret = toStringList();

  //we always want to have the current value available
  if (!isEnumValue(src)) {
    ret.push_back(src);
  }

  return ret;
}

template<typename T>
QStringList Enum<T>::getQStringList() const
{
  auto ret = toQStringList();

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

