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
    static constexpr std::string_view UNUSED = "<<empty>>"; //don't export this node
    static constexpr std::string_view ACTION_POSTFIX = "_Hold";

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
      return getActionId(device.get(), action.get());
    }
    static std::string getActionId(const std::string &device, const std::string &action)
    {
      return device + "_" + action + std::string(ACTION_POSTFIX);
    }

    /** for import */
    static std::string getAction(const std::string &aid);
    /** for import */
    static uint32_t getDevice(const std::string &aid);

    /** command that will be sent on press. Device -- name, Activity -- ID_name */
    PropertyString action{"act"};
    /** only activity: device ID that contains command */
    PropertyU32 device{ 0, Used::NO} ;
    /** hard button: name, soft button: label */
    PropertyString name{"Generic", Used::NO};
    /** only soft button: button icon */
    PropertyString file{"0.png", Used::NO};
    /** only soft button: position on lcd, starting at 0. 6 devices per page, so 7 -> pos 0 on second page */
    PropertyI32 position{-1, Used::NO};
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
