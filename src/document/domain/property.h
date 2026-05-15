// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <string>
#include <QString>

namespace document
{
namespace domain
{

/** dump to export file */
enum class Include
{
  ALWAYS, OPTIONAL,
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
    Property(T v, Include defaultIncluded = Include::OPTIONAL) :
        include(defaultIncluded), value(v)
    {
    }

    void setIncluded(Include i)
    {
      include = i;
    }

    Include isIncluded() const
    {
      return include;
    }

    T getValue() const
    {
      return value;
    }

    void setValue(T v)
    {
      value = v;
    }

  protected:
    Include include;
    T value;
};

}
}

//fixme -- don't use a preprocessor macro...
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
