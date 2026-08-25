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

    /** copy / new uid */
    Device(const Device &other, uint32_t newId) :
        Device(other)
    {
      id = newId;
    }

    const std::vector<Button>& getSoftButtons() const
    {
      return softButtons;
    }

    std::vector<Button>& getSoftButtons()
    {
      return softButtons;
    }

    const std::vector<Button>& getHardButtons() const
    {
      return hardButtons;
    }

    std::vector<Button>& getHardButtons()
    {
      return hardButtons;
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
    PropertyBool hasBands{false, Used::NO};
    PropertyBool hasPresets{false, Used::NO};
    PropertyBool isNewDevice{false, Used::NO};
    PropertyBool isDisplayDevice{false, Used::NO};
    PropertyBool menuOnDevice{false, Used::NO};
    PropertyI32 numDiscs{1, Used::NO};
    PropertyI32 numLights{1, Used::NO};
    PropertyBool onScreenGuide{false, Used::NO};
    PropertyEnum<PvrType> pvrType{PvrType::Generic, Used::NO};
    PropertyBool recordMediaFixedDisc{false, Used::NO};
    PropertyBool recordMediaRemovableVideotape{false, Used::NO};
    PropertyBool revertInput{false, Used::NO};
    PropertyBool scart{false, Used::NO};
    PropertyEnum<TunerInput> tunerInput{TunerInput::Tuner, Used::NO};
    PropertyBool videoSwitch{false, Used::NO};

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return unknownProperties;
    }

    std::vector<UnknownElement> &getUnknownProperties()
    {
      return unknownProperties;
    }

  protected:
    std::vector<Button> softButtons;
    std::vector<Button> hardButtons;
    std::vector<StateMachine> stateMachines;
    std::optional<Numpad> numpad;
    Commands irCommands;

    uint32_t id;
    std::vector<UnknownElement> unknownProperties;
};

}
}
}
