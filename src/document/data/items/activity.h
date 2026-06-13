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

/** stores one single activity info struct from UserConfiguration.xml
 */
class Activity
{
  public:
    Activity(uint32_t id, bool powerOff = false);

    uint32_t getId() const
    {
      return id;
    }

    PROPERTY_GETTER(Property<Enum<ActivityStartPage>>, pvrType, getPvrType)
    PROPERTY_GETTER(Property<bool>, controlGroup_HardButtons, getControlGroup_HardButtons)
    PROPERTY_GETTER(Property<bool>, powerOffUnusedDevices, getPowerOffUnusedDevices)
    PROPERTY_GETTER(Property<bool>, trainingWheels, getTrainingWheels)
    PROPERTY_GETTER(Property<bool>, unusedDevicesHelp, getUnusedDevicesHelp)
    PROPERTY_GETTER(Property<Enum<ChannelButtonBehaviour>>, channelButtonBehaviour, getChannelButtonBehaviour)
    PROPERTY_GETTER(Property<bool>, controlGroup_SoftButtons, getControlGroup_SoftButtons)
    PROPERTY_GETTER(Property<bool>, enableSmartMenu, getEnableSmartMenu)
    PROPERTY_GETTER(Property<bool>, enableSmartZoom, getEnableSmartZoom)
    PROPERTY_GETTER(Property<Enum<GuideButtonMode>>, guideButtonMode, getGuideButtonMode)
    PROPERTY_GETTER(Property<bool>, hideModeControl, getHideModeControl)
    PROPERTY_GETTER(Property<bool>, hideModeListen, getHideModeListen)
    PROPERTY_GETTER(Property<bool>, hideModeNavigate, getHideModeNavigate)
    PROPERTY_GETTER(Property<bool>, hideModePlay, getHideModePlay)
    PROPERTY_GETTER(Property<bool>, hideModePlayMode, getHideModePlayMode)
    PROPERTY_GETTER(Property<bool>, hideSurfAllChannels, getHideSurfAllChannels)
    PROPERTY_GETTER(Property<bool>, hideSurfAllShows, getHideSurfAllShows)
    PROPERTY_GETTER(Property<bool>, hideSurfFavoriteChannels, getHideSurfFavoriteChannels)
    PROPERTY_GETTER(Property<bool>, hideSurfFavoriteShows, getHideSurfFavoriteShows)
    PROPERTY_GETTER(Property<uint32_t>, maxTvContentDays, getMaxTvContentDays)
    PROPERTY_GETTER(Property<Enum<MediaButtonMode>>, mediaButtonMode, getMediaButtonMode)
    PROPERTY_GETTER(Property<bool>, playOnEnter, getPlayOnEnter)
    PROPERTY_GETTER(Property<bool>, retainStop, getRetainStop)
    PROPERTY_GETTER(Property<bool>, scrollChannelsByPage, getScrollChannelsByPage)
    PROPERTY_GETTER(Property<bool>, scrollShowsByPage, getScrollShowsByPage)
    PROPERTY_GETTER(Property<bool>, stopOnExit, getStopOnExit)

    //todo getter/setter

    const std::vector<UnknownElement> &getUnknownItems() const
    {
      return u;
    }

    void setUnknownItems(const std::vector<UnknownElement> &u)
    {
      this->u = u;
    }

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return u;
    }

    void setUnknownProperties(const std::vector<UnknownElement> &u)
    {
      p.u = u;
    }

  protected:
    struct Properties {
      //general
      Property<Enum<ActivityStartPage>> pvrType{ActivityStartPage::Transport, Include::ALWAYS};
      Property<bool> controlGroup_HardButtons{true, Include::ALWAYS};
      Property<bool> powerOffUnusedDevices{true, Include::ALWAYS};
      Property<bool> trainingWheels{true, Include::ALWAYS};
      Property<bool> unusedDevicesHelp{false, Include::ALWAYS};
      //other
      Property<Enum<ChannelButtonBehaviour>> channelButtonBehaviour{ChannelButtonBehaviour::BasicChannels, Include::CHECK};
      Property<bool> controlGroup_SoftButtons{true, Include::CHECK};
      Property<bool> enableSmartMenu{true, Include::CHECK};
      Property<bool> enableSmartZoom{true, Include::CHECK};
      Property<Enum<GuideButtonMode>> guideButtonMode{GuideButtonMode::TunerProgramGuide, Include::CHECK};
      Property<bool> hideModeControl{false, Include::CHECK};
      Property<bool> hideModeListen{false, Include::CHECK};
      Property<bool> hideModeNavigate{false, Include::CHECK};
      Property<bool> hideModePlay{false, Include::CHECK};
      Property<bool> hideModePlayMode{false, Include::CHECK};
      Property<bool> hideSurfAllChannels{false, Include::CHECK};
      Property<bool> hideSurfAllShows{false, Include::CHECK};
      Property<bool> hideSurfFavoriteChannels{false, Include::CHECK};
      Property<bool> hideSurfFavoriteShows{false, Include::CHECK};
      Property<uint32_t> maxTvContentDays{false, Include::CHECK};
      Property<Enum<MediaButtonMode>> mediaButtonMode{MediaButtonMode::ShowMedia, Include::CHECK};
      Property<bool> playOnEnter{true, Include::CHECK};
      Property<bool> retainStop{false, Include::CHECK};
      Property<bool> scrollChannelsByPage{true, Include::CHECK};
      Property<bool> scrollShowsByPage{true, Include::CHECK};
      Property<bool> stopOnExit{false, Include::CHECK};

      //other other
      std::vector<UnknownElement> u;
    } p;

    bool powerOff; //has no id, no properties
    const uint32_t id;
    Enum<ActivityType> type{ActivityType::VirtualGeneric};
    std::vector<Button> softButtons;
    std::vector<Button> hardButtons;
    std::vector<StateAction> enterActions;
    std::vector<StateAction> leaveActions;
    std::vector<uint32_t> powerOnDeviceIds;
    std::vector<uint32_t> powerOffDeviceIds;
    std::vector<Role> roles;

    std::vector<UnknownElement> u;
};

}
}
}
