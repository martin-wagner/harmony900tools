// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

inline item::DeviceAction* getActionFromSmRef(ConfigData &c, uint32_t devicePos,
    uint32_t smPos, item::StateTransitionAction t, uint32_t actPos)
{
  try {
    auto &sm = c.getDevices().at(devicePos).getStateMachines().at(smPos);

    switch (t) {
      case item::StateTransitionAction::Discrete_Enter:
        return &sm.discrete.enterStateAction.at(actPos);
      case item::StateTransitionAction::Relative_Reset:
        if (sm.relative.resetAction != std::nullopt) {
          return &sm.relative.resetAction.value();
        }
        break;
      case item::StateTransitionAction::Relative_Next:
        if (sm.relative.nextStateAction != std::nullopt) {
          return &sm.relative.nextStateAction.value();
        }
        break;
      case item::StateTransitionAction::Relative_Prev:
        if (sm.relative.prevStateAction != std::nullopt) {
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
    if (num == std::nullopt) {
      return nullptr;
    }

    switch (s) {
      case item::DigitSection::First:
        if (num->first != std::nullopt) {
          return &num->first->at(digit);
        }
        break;
      case item::DigitSection::Middle:
        if (num->middle != std::nullopt) {
          return &num->middle->at(digit);
        }
        break;
      case item::DigitSection::Last:
        if (num->last != std::nullopt) {
          return &num->last->at(digit);
        }
        break;
      case item::DigitSection::Finish:
        if (num->finish != std::nullopt) {
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

}
}
