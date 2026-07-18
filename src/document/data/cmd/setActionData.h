// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

inline item::DeviceAction* getActionFromSmRef(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateMachineAction t, uint32_t actPos)
{
  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);

    switch (t) {
      case item::StateMachineAction::Start:
        if (sm.startAction.has_value()) {
          return &sm.startAction.value();
        }
        break;
      case item::StateMachineAction::Finish:
        if (sm.finishAction.has_value()) {
          return &sm.finishAction.value();
        }
        break;
      case item::StateMachineAction::Discrete_Enter:
        return &sm.discrete.enterStateAction.at(actPos);
      case item::StateMachineAction::Relative_Reset:
        if (sm.relative.resetAction.has_value()) {
          return &sm.relative.resetAction.value();
        }
        break;
      case item::StateMachineAction::Relative_Next:
        if (sm.relative.nextStateAction.has_value()) {
          return &sm.relative.nextStateAction.value();
        }
        break;
      case item::StateMachineAction::Relative_Prev:
        if (sm.relative.prevStateAction.has_value()) {
          return &sm.relative.prevStateAction.value();
        }
        break;
      default:
        break;
    }
  } catch (std::out_of_range&) {
  }
  return nullptr;
}

inline item::DeviceAction* getActionFromNumpadRef(ConfigData &c,
    uint32_t devicePos, item::DigitSection s, uint32_t digit)
{
  try {
    auto &num = c.getDevices().at(devicePos).getNumpad();
    if (!num.has_value()) {
      return nullptr;
    }

    switch (s) {
      case item::DigitSection::First:
        if (num->first.has_value()) {
          return &num->first->at(digit);
        }
        break;
      case item::DigitSection::Middle:
        if (num->middle.has_value()) {
          return &num->middle->at(digit);
        }
        break;
      case item::DigitSection::Last:
        if (num->last.has_value()) {
          return &num->last->at(digit);
        }
        break;
      case item::DigitSection::Finish:
        if (num->finish.has_value()) {
          return &num->finish.value();
        }
        break;
      default:
        break;
    }
  } catch (std::out_of_range&) {
  }
  return nullptr;
}

inline item::DeviceAction* getActionFromActivity(ConfigData &c,
    uint32_t activityPos, item::ActivityAction t, uint32_t actionPos)
{
  try {
    auto &activity = c.getActivities().at(activityPos);
    switch (t) {
      case item::ActivityAction::Enter:
        return &activity.getEnterActions().at(actionPos);
        break;
      case item::ActivityAction::Leave:
        return &activity.getLeaveActions().at(actionPos);
        break;
      default:
        return nullptr;
    }
  } catch (std::out_of_range&) {
  }
  return nullptr;
}

}
}
