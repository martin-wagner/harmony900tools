// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

inline item::Button* getDeviceButton(ConfigData &c, uint32_t devicePos, item::ButtonType t, int buttonPos)
{
  try {
    auto &device = c.getDevices().at(devicePos);

    switch (t) {
      case item::ButtonType::Hard:
        return &device.getHardButtons().at(buttonPos);
      case item::ButtonType::Soft:
        return &device.getSoftButtons().at(buttonPos);
      default:
        break;
    }
  } catch (std::out_of_range&) {
  }
  return nullptr;
}

Item toDeviceItem(item::ButtonType t)
{
  switch (t) {
    case item::ButtonType::Hard:
      return Item::DEVICE_HARD_BUTTON;
    default:
      return Item::DEVICE_SOFT_BUTTON;
  }
}

inline item::Button* getActivitiesButton(ConfigData &c, uint32_t activityPos, item::ButtonType t, int buttonPos)
{
  try {
    auto &act = c.getActivities().at(activityPos);

    switch (t) {
      case item::ButtonType::Hard:
        return &act.getHardButtons().at(buttonPos);
      case item::ButtonType::Soft:
        return &act.getSoftButtons().at(buttonPos);
      default:
        break;
    }
  } catch (std::out_of_range&) {
  }
  return nullptr;
}

Item toActivityItem(item::ButtonType t)
{
  switch (t) {
    case item::ButtonType::Hard:
      return Item::ACTIVITY_HARD_BUTTON;
    default:
      return Item::ACTIVITY_SOFT_BUTTON;
  }
}

}
}
