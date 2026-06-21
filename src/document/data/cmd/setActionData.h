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
        if (sm.relative.resetAction) {
          return &sm.relative.resetAction.value();
        }
        break;
      case item::StateTransitionAction::Relative_Next:
        if (sm.relative.nextStateAction) {
          return &sm.relative.nextStateAction.value();
        }
        break;
      case item::StateTransitionAction::Relative_Prev:
        if (sm.relative.prevStateAction) {
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

}
}
