// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <set>
#include <chrono>

#include "document/data/enum.h"
#include "document/data/property.h"
#include "unknown.h"
#include "state.h"
#include "digits.h"
#include "commands.h"
#include "button.h"

namespace document
{
namespace data
{
namespace item
{

/** stores one single device info struct from UserConfiguration.xml
 *
 * some parsing in usr/local/share/lua/5.1/ethanol/objects/Device.lua
 */
class Device
{
  public:
    Device(uint32_t id) : id(id) {};

    /** get device uid */
    uint32_t getId() const
    {
      return id;
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

    const std::vector<StateMachine>& getStateMachines() const
    {
      return stateMachines;
    }

    std::vector<StateMachine>& getStateMachines()
    {
      return stateMachines;
    }

    const std::optional<Numpad>& getNumpad() const
    {
      return numpad;
    }

    std::optional<Numpad>& getNumpad()
    {
      return numpad;
    }

    const Commands& getIrCommands() const
    {
      return irCommands;
    }

    Commands& getIrCommands()
    {
      return irCommands;
    }

    void setIrCommands(const Commands &irCommands)
    {
      this->irCommands = irCommands;
    }

    PropertyEnum<DeviceType> type{DeviceType::Computer};
    PropertyString mnf{"Commodore"};
    PropertyString model{"C64"};
    PropertyString label{"My C64"};

    //power related
    PropertyBool manualPower{false, Used::YES};
    PropertyBool alwaysOn{false, Used::NO};
    PropertyBool autoPower{false, Used::NO};
    //other
    PropertyBool audioSwitch{false, Used::NO};
    PropertyBool dimmer{false, Used::NO};
    PropertyBool hasBands{true, Used::NO};
    PropertyBool hasPresets{true, Used::NO};
    PropertyBool isNewDevice{true, Used::NO};
    PropertyBool isDisplayDevice{true, Used::NO};
    PropertyBool menuOnDevice{false, Used::NO};
    PropertyI32 numDiscs{1, Used::NO};
    PropertyI32 numLights{1, Used::NO};
    PropertyBool onScreenGuide{false, Used::NO};
    PropertyEnum<PvrType> pvrType{PvrType::Generic, Used::NO};
    PropertyBool recordMediaFixedDisc{true, Used::NO};
    PropertyBool recordMediaRemovableVideotape{true, Used::NO};
    PropertyBool revertInput{true, Used::NO};
    PropertyBool scart{true, Used::NO};
    PropertyEnum<TunerInput> tunerInput{TunerInput::Tuner, Used::NO};
    PropertyBool videoSwitch{true, Used::NO};

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return unknownProperties;
    }

    std::vector<UnknownElement> &getUnknownProperties()
    {
      return unknownProperties;
    }

  protected:
    std::vector<Button> buttons;
    std::vector<StateMachine> stateMachines;
    std::optional<Numpad> numpad;
    Commands irCommands;

    uint32_t id;
    std::vector<UnknownElement> unknownProperties;
};

}
}
}
