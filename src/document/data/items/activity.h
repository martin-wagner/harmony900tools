// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>

#include "document/data/enum.h"
#include "document/data/property.h"
#include "unknown.h"
#include "button.h"
#include "state.h"
#include "digits.h"
#include "commands.h"
#include "action.h"
#include "role.h"

namespace document
{
namespace data
{
namespace item
{

enum class ActivityAction {
    Unknown,
    Enter,
    Leave
};

/** stores one single activity info struct from UserConfiguration.xml
 *
 * some parsing in usr/local/share/lua/5.1/ethanol/objects/Activity.lua
 */
class Activity
{
  public:
    Activity(uint32_t id) : id(id)
    {
    }

    uint32_t getId() const
    {
      return id;
    }

    const std::vector<Channel>& getChannels() const
    {
      return channels;
    }

    std::vector<Channel>& getChannels()
    {
      return channels;
    }

    const std::vector<Button>& getButtons() const
    {
      return buttons;
    }

    std::vector<Button>& getButtons()
    {
      return buttons;
    }

    bool hasSoftButtons() const
    {
      for (const auto &b : buttons) {
        if (b.getButtonType() == ButtonType::Soft) {
          return true;
        }
      }
      return false;
    }

    bool hasHardButtons() const
    {
      for (const auto &b : buttons) {
        if (b.getButtonType() == ButtonType::Hard) {
          return true;
        }
      }
      return false;
    }

    const std::vector<DeviceAction>& getEnterActions() const
    {
      return enterActions;
    }

    std::vector<DeviceAction>& getEnterActions()
    {
      return enterActions;
    }

    const std::vector<DeviceAction>& getLeaveActions() const
    {
      return leaveActions;
    }

    std::vector<DeviceAction>& getLeaveActions()
    {
      return leaveActions;
    }

    const std::vector<Role>& getRoles() const
    {
      return roles;
    }

    std::vector<Role>& getRoles()
    {
      return roles;
    }

    const std::vector<uint32_t>& getPowerOnDevices() const
    {
      return powerOnDeviceIds;
    }

    std::vector<uint32_t>& getPowerOnDevices()
    {
      return powerOnDeviceIds;
    }

    const std::vector<uint32_t>& getPowerOffDevices() const
    {
      return powerOffDeviceIds;
    }

    std::vector<uint32_t>& getPowerOffDevices()
    {
      return powerOffDeviceIds;
    }

    PropertyEnum<ActivityType> type{ActivityType::VirtualGeneric};
    PropertyString label{"My C64"};

    //general
    PropertyEnum<ActivityStartPage> pvrType{ActivityStartPage::Transport, Used::YES};
    PropertyBool controlGroup_HardButtons{true, Used::YES};
    PropertyBool powerOffUnusedDevices{true, Used::YES};
    PropertyBool trainingWheels{true, Used::YES};
    PropertyBool unusedDevicesHelp{false, Used::YES};

    //other
    PropertyEnum<ChannelButtonBehaviour> channelButtonBehaviour{ChannelButtonBehaviour::BasicChannels, Used::NO};
    PropertyBool controlGroup_SoftButtons{true, Used::NO};
    PropertyBool enableSmartMenu{true, Used::NO};
    PropertyBool enableSmartZoom{true, Used::NO};
    PropertyEnum<GuideButtonMode> guideButtonMode{GuideButtonMode::TunerProgramGuide, Used::NO};
    PropertyBool hideModeControl{false, Used::NO};
    PropertyBool hideModeListen{false, Used::NO};
    PropertyBool hideModeNavigate{false, Used::NO};
    PropertyBool hideModePlay{false, Used::NO};
    PropertyBool hideModePlayMode{false, Used::NO};
    PropertyBool hideSurfAllChannels{false, Used::NO};
    PropertyBool hideSurfAllShows{false, Used::NO};
    PropertyBool hideSurfFavoriteChannels{false, Used::NO};
    PropertyBool hideSurfFavoriteShows{false, Used::NO};
    PropertyI32 maxTvContentDays{0, Used::NO};
    PropertyEnum<MediaButtonMode> mediaButtonMode{MediaButtonMode::ShowMedia, Used::NO};
    PropertyBool playOnEnter{true, Used::NO};
    PropertyBool retainStop{false, Used::NO};
    PropertyBool scrollChannelsByPage{true, Used::NO};
    PropertyBool scrollShowsByPage{true, Used::NO};
    PropertyBool stopOnExit{false, Used::NO};

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return unknownProperties;
    }

    std::vector<UnknownElement> &getUnknownProperties()
    {
      return unknownProperties;
    }

  protected:
    uint32_t id;
    std::vector<Channel> channels;
    std::vector<Button> buttons;
    std::vector<DeviceAction> enterActions;
    std::vector<DeviceAction> leaveActions;
    std::vector<Role> roles;
    std::vector<uint32_t> powerOnDeviceIds;
    std::vector<uint32_t> powerOffDeviceIds;
    //other devices keep their state on activity change

    std::vector<UnknownElement> unknownProperties;

};

}
}
}
