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

/** stores meta info for a function button
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

    /** for export device */
    const std::string getActionId(uint32_t id) const
    {
      return getActionId(id, action.get());
    }
    static std::string getActionId(uint32_t id, const std::string &action)
    {
      return std::to_string(id) + "_" + action + std::string(ACTION_POSTFIX);
    }
    /** for export activity */
    const std::string getActionId() const
    {
      return getActionId(action.get());
    }
    static std::string getActionId(const std::string &action)
    {
      return action + std::string(ACTION_POSTFIX);
    }

    /** for import */
    static std::string getActionDevice(const std::string &aid);
    /** for import */
    static std::string getActionActivity(const std::string &aid);

    /** command that will be sent on press. Device -- name, Activity -- ID_name */
    PropertyString action{"act"};
    /** hard button: name, soft button: label */
    PropertyString name{"Generic", Include::CHECK};
    /** only soft button: channel icon */
    PropertyString file{"0.png", Include::CHECK};
    /** only soft button: position on lcd, starting at 0. 6 devices per page, so 7 -> pos 0 on second page */
    PropertyI32 position{0, Include::CHECK};

  protected:
    ButtonType type;
};

/** stores meta info for a channel button (activities)
 */
class Channel
{
  public:
    /** command that will be sent on press */
    PropertyString station{"act"};
    /** channel number (what you enter on the remote to select a channel) */
    PropertyU32 channel{1};
    /** position on lcd, starting at 0. 6 devices per page, so 7 -> pos 0 on second page */
    PropertyU32 position{0};
    /** channel image name, image in /image/xxxx.png, PNG image data, 89 x 55, 8-bit/color RGB, non-interlaced */
    PropertyString img{"img.png"};
};

}
}
}
