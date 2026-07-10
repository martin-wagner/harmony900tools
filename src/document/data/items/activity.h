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
    PropertyEnum<ActivityStartPage> pvrType{ActivityStartPage::Transport, Include::ALWAYS};
    PropertyBool controlGroup_HardButtons{true, Include::ALWAYS};
    PropertyBool powerOffUnusedDevices{true, Include::ALWAYS};
    PropertyBool trainingWheels{true, Include::ALWAYS};
    PropertyBool unusedDevicesHelp{false, Include::ALWAYS};

    //other
    PropertyEnum<ChannelButtonBehaviour> channelButtonBehaviour{ChannelButtonBehaviour::BasicChannels, Include::CHECK};
    PropertyBool controlGroup_SoftButtons{true, Include::CHECK};
    PropertyBool enableSmartMenu{true, Include::CHECK};
    PropertyBool enableSmartZoom{true, Include::CHECK};
    PropertyEnum<GuideButtonMode> guideButtonMode{GuideButtonMode::TunerProgramGuide, Include::CHECK};
    PropertyBool hideModeControl{false, Include::CHECK};
    PropertyBool hideModeListen{false, Include::CHECK};
    PropertyBool hideModeNavigate{false, Include::CHECK};
    PropertyBool hideModePlay{false, Include::CHECK};
    PropertyBool hideModePlayMode{false, Include::CHECK};
    PropertyBool hideSurfAllChannels{false, Include::CHECK};
    PropertyBool hideSurfAllShows{false, Include::CHECK};
    PropertyBool hideSurfFavoriteChannels{false, Include::CHECK};
    PropertyBool hideSurfFavoriteShows{false, Include::CHECK};
    PropertyI32 maxTvContentDays{0, Include::CHECK};
    PropertyEnum<MediaButtonMode> mediaButtonMode{MediaButtonMode::ShowMedia, Include::CHECK};
    PropertyBool playOnEnter{true, Include::CHECK};
    PropertyBool retainStop{false, Include::CHECK};
    PropertyBool scrollChannelsByPage{true, Include::CHECK};
    PropertyBool scrollShowsByPage{true, Include::CHECK};
    PropertyBool stopOnExit{false, Include::CHECK};

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
