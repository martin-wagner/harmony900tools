// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <string>
#include <QString>

namespace document
{
namespace data
{

/** value is in use / can be ignored on save / export */
enum class Used
{
  YES, NO
};

/**
 * stores one property from the properties list
 *
 * string allows to dump original value if available
 */
template<typename T>
class Property
{
  public:
    Property(const T &v, Used defaultIncluded = Used::YES) :
        include(defaultIncluded), value(v)
    {
    }

    void setIncluded(Used i)
    {
      include = i;
    }

    Used isIncluded() const
    {
      return include;
    }

    T get() const
    {
      return value;
    }

    Property& set(const T &v)
    {
      value = v;
      return *this;
    }

  protected:
    Used include;
    T value;
};

//some common use cases...
using PropertyString = Property<std::string>;
using PropertyBool = Property<bool>;
using PropertyU8 = Property<uint8_t>;
using PropertyU16 = Property<uint16_t>;
using PropertyI32 = Property<int32_t>;
using PropertyU32 = Property<uint32_t>;
template<typename T>
using PropertyEnum = Property<Enum<T>>;

}
}

// todo alle properties using ... verwenden, das hier wegmachen
#define PROPERTY_GETTER(type, varname, methodname) \
  type& methodname()                       \
  {                                        \
    return p.varname;                      \
  }                                        \
                                           \
  const type& methodname() const           \
  {                                        \
    return p.varname;                      \
  }

