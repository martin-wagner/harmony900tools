// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace document
{
namespace domain
{
namespace item
{

/** stores imported element that are not known at software build time.
 *
 * needed so we can dump them into the exported file untouched.
 */
class UnknownElement
{
  public:
    UnknownElement() {};
    UnknownElement(const std::string &tag,
        const std::map<std::string, std::string> &attributes,
        const std::string &text, const std::vector<UnknownElement> &children =
            std::vector<UnknownElement>()) :
        tag(tag), attributes(attributes), text(text), children(children)
    {
    }

    std::string tag;
    std::map<std::string, std::string> attributes;
    std::string text;
    std::vector<UnknownElement> children;
};

}
}
}
