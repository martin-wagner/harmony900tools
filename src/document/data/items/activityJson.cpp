// SPDX-License-Identifier: LGPL-2.1-or-later

#include "deviceJson.h"
#include "activityJson.h"

#include "jsonSerialise.h"

using namespace std;

namespace document
{
namespace data
{
namespace serialiser
{

//---------------------------------------------------------------------------
// button.h (Channel)
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::Channel &channel)
{
  toJson(out, "Station", channel.station);
  toJson(out, "Channel", channel.channel);
  toJson(out, "Position", channel.position);
  toJson(out, "Img", channel.img);
}

void fromJson(const ordered_json &in, item::Channel &channel)
{
  fromJson(in, "Station", channel.station);
  fromJson(in, "Channel", channel.channel);
  fromJson(in, "Position", channel.position);
  fromJson(in, "Img", channel.img);
}

//---------------------------------------------------------------------------
// role.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::Role &role)
{
  toJson(out, "DeviceId", role.deviceId);
  toJson(out, "Role", role.role);
}

void fromJson(const ordered_json &in, item::Role &role)
{
  fromJson(in, "DeviceId", role.deviceId);
  fromJson(in, "Role", role.role);
}

//---------------------------------------------------------------------------
// activity.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::Activity &activity)
{
  ordered_json softButtonsArr = ordered_json::array();
  ordered_json hardButtonsArr = ordered_json::array();
  ordered_json buttonsObj;

  out["Id"] = activity.getId();

  toJsonVec(out, "Channels", activity.getChannels());
  for (const auto &button : activity.getButtons()) {
    ordered_json buttonJson;
    toJson(buttonJson, button);

    if (button.getButtonType() == item::ButtonType::Soft) {
      softButtonsArr.push_back(buttonJson);
    } else {
      hardButtonsArr.push_back(buttonJson);
    }
  }
  if (!softButtonsArr.empty()) {
    buttonsObj["Soft"] = softButtonsArr;
  }
  if (!hardButtonsArr.empty()) {
    buttonsObj["Hard"] = hardButtonsArr;
  }
  if (!buttonsObj.empty()) {
    out["Buttons"] = buttonsObj;
  }
  toJsonVec(out, "EnterActions", activity.getEnterActions());
  toJsonVec(out, "LeaveActions", activity.getLeaveActions());
  toJsonVec(out, "Roles", activity.getRoles());

  if (!activity.getPowerOnDevices().empty()) {
    out["PowerOnDeviceIds"] = activity.getPowerOnDevices();
  }
  if (!activity.getPowerOffDevices().empty()) {
    out["PowerOffDeviceIds"] = activity.getPowerOffDevices();
  }

  toJson(out, "Type", activity.type);
  toJson(out, "Label", activity.label);

  toJson(out, "PvrType", activity.pvrType);
  toJson(out, "ControlGroup_HardButtons", activity.controlGroup_HardButtons);
  toJson(out, "PowerOffUnusedDevices", activity.powerOffUnusedDevices);
  toJson(out, "TrainingWheels", activity.trainingWheels);
  toJson(out, "UnusedDevicesHelp", activity.unusedDevicesHelp);

  toJson(out, "ChannelButtonBehaviour", activity.channelButtonBehaviour);
  toJson(out, "ControlGroup_SoftButtons", activity.controlGroup_SoftButtons);
  toJson(out, "EnableSmartMenu", activity.enableSmartMenu);
  toJson(out, "EnableSmartZoom", activity.enableSmartZoom);
  toJson(out, "GuideButtonMode", activity.guideButtonMode);
  toJson(out, "HideModeControl", activity.hideModeControl);
  toJson(out, "HideModeListen", activity.hideModeListen);
  toJson(out, "HideModeNavigate", activity.hideModeNavigate);
  toJson(out, "HideModePlay", activity.hideModePlay);
  toJson(out, "HideModePlayMode", activity.hideModePlayMode);
  toJson(out, "HideSurfAllChannels", activity.hideSurfAllChannels);
  toJson(out, "HideSurfAllShows", activity.hideSurfAllShows);
  toJson(out, "HideSurfFavoriteChannels", activity.hideSurfFavoriteChannels);
  toJson(out, "HideSurfFavoriteShows", activity.hideSurfFavoriteShows);
  toJson(out, "MaxTvContentDays", activity.maxTvContentDays);
  toJson(out, "MediaButtonMode", activity.mediaButtonMode);
  toJson(out, "PlayOnEnter", activity.playOnEnter);
  toJson(out, "RetainStop", activity.retainStop);
  toJson(out, "ScrollChannelsByPage", activity.scrollChannelsByPage);
  toJson(out, "ScrollShowsByPage", activity.scrollShowsByPage);
  toJson(out, "StopOnExit", activity.stopOnExit);

  toJson(out, "UnknownProperties", activity.getUnknownProperties());
}

void fromJson(const ordered_json &in, item::Activity &activity)
{
  auto readButtons = [&activity](const ordered_json &in,
      item::ButtonType type) {
        for (const auto &buttonJson : in) {
          item::Button button(type);
          fromJson(buttonJson, button);
          activity.getButtons().push_back(button);
        }
      };

  fromJson(in, "Type", activity.type);
  fromJson(in, "Label", activity.label);

  fromJson(in, "PvrType", activity.pvrType);
  fromJson(in, "ControlGroup_HardButtons", activity.controlGroup_HardButtons);
  fromJson(in, "PowerOffUnusedDevices", activity.powerOffUnusedDevices);
  fromJson(in, "TrainingWheels", activity.trainingWheels);
  fromJson(in, "UnusedDevicesHelp", activity.unusedDevicesHelp);

  fromJson(in, "ChannelButtonBehaviour", activity.channelButtonBehaviour);
  fromJson(in, "ControlGroup_SoftButtons", activity.controlGroup_SoftButtons);
  fromJson(in, "EnableSmartMenu", activity.enableSmartMenu);
  fromJson(in, "EnableSmartZoom", activity.enableSmartZoom);
  fromJson(in, "GuideButtonMode", activity.guideButtonMode);
  fromJson(in, "HideModeControl", activity.hideModeControl);
  fromJson(in, "HideModeListen", activity.hideModeListen);
  fromJson(in, "HideModeNavigate", activity.hideModeNavigate);
  fromJson(in, "HideModePlay", activity.hideModePlay);
  fromJson(in, "HideModePlayMode", activity.hideModePlayMode);
  fromJson(in, "HideSurfAllChannels", activity.hideSurfAllChannels);
  fromJson(in, "HideSurfAllShows", activity.hideSurfAllShows);
  fromJson(in, "HideSurfFavoriteChannels", activity.hideSurfFavoriteChannels);
  fromJson(in, "HideSurfFavoriteShows", activity.hideSurfFavoriteShows);
  fromJson(in, "MaxTvContentDays", activity.maxTvContentDays);
  fromJson(in, "MediaButtonMode", activity.mediaButtonMode);
  fromJson(in, "PlayOnEnter", activity.playOnEnter);
  fromJson(in, "RetainStop", activity.retainStop);
  fromJson(in, "ScrollChannelsByPage", activity.scrollChannelsByPage);
  fromJson(in, "ScrollShowsByPage", activity.scrollShowsByPage);
  fromJson(in, "StopOnExit", activity.stopOnExit);

  fromJson(in, "UnknownProperties", activity.getUnknownProperties());

  //note: activity.id is set at construction time by the caller (Activity has
  //no default constructor / setter), so it is not read back here.

  fromJsonVec(in, "Channels", activity.getChannels());
  auto buttonsIt = in.find("Buttons");
  if (buttonsIt != in.end()) {
    auto softIt = buttonsIt->find("Soft");
    if (softIt != buttonsIt->end()) {
      readButtons(*softIt, item::ButtonType::Soft);
    }

    auto hardIt = buttonsIt->find("Hard");
    if (hardIt != buttonsIt->end()) {
      readButtons(*hardIt, item::ButtonType::Hard);
    }
  }
  fromJsonVec(in, "EnterActions", activity.getEnterActions());
  fromJsonVec(in, "LeaveActions", activity.getLeaveActions());
  fromJsonVec(in, "Roles", activity.getRoles());

  activity.getPowerOnDevices() = in.value("PowerOnDeviceIds",
      std::vector<uint32_t>());
  activity.getPowerOffDevices() = in.value("PowerOffDeviceIds",
      std::vector<uint32_t>());
}

}
}
}
