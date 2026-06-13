// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>

#include "document/data/enum.h"
#include "document/data/property.h"

namespace document
{
namespace data
{
namespace item
{

enum class ButtonType { Soft, Hard };

/** stores meta info for a button
 */
class Button
{
  public:
    static constexpr std::string_view ACTION_POSTFIX = "_Hold";

    Button(ButtonType t) : type(t)
    {
    }

    ButtonType getButtonType() const
    {
      return type;
    }

    /** for export */
    const std::string getActionId(uint32_t id) const
    {
      return getActionId(id, action.get());
    }
    static std::string getActionId(uint32_t id, const std::string &action)
    {
      return std::to_string(id) + "_" + action + std::string(ACTION_POSTFIX);
    }

    /** for import */
    static std::string getAction(const std::string &aid);

    /** command that will be sent on press */
    PropertyString action{"act"};
    /** hard button: name, soft button: label */
    PropertyString name{"Generic"};
    /** only soft button: channel icon */
    PropertyString file{"0.png", Include::CHECK};
    /** only soft button: position on lcd, starting at 0. 6 devices per page, so 7 -> pos 0 on second page */
    PropertyI32 position{0, Include::CHECK};

  protected:
    ButtonType type;
};

}
}
}
