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

    /** get device uid + all command uids */
    std::set<uint32_t> getAllIds() { return {}; }; //todo

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
    PropertyBool manualPower{false, Include::ALWAYS};
    PropertyBool alwaysOn{false, Include::CHECK};
    PropertyBool autoPower{false, Include::CHECK};
    //other
    PropertyBool audioSwitch{false, Include::CHECK};
    PropertyBool dimmer{false, Include::CHECK};
    PropertyBool hasBands{true, Include::CHECK};
    PropertyBool hasPresets{true, Include::CHECK};
    PropertyBool isNewDevice{true, Include::CHECK};
    PropertyBool isDisplayDevice{true, Include::CHECK};
    PropertyBool menuOnDevice{false, Include::CHECK};
    PropertyI32 numDiscs{1, Include::CHECK};
    PropertyI32 numLights{1, Include::CHECK};
    PropertyBool onScreenGuide{false, Include::CHECK};
    PropertyEnum<PvrType> pvrType{PvrType::Generic, Include::CHECK};
    PropertyBool recordMediaFixedDisc{true, Include::CHECK};
    PropertyBool recordMediaRemovableVideotape{true, Include::CHECK};
    PropertyBool revertInput{true, Include::CHECK};
    PropertyBool scart{true, Include::CHECK};
    PropertyEnum<TunerInput> tunerInput{TunerInput::Tuner, Include::CHECK};
    PropertyBool videoSwitch{true, Include::CHECK};

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
    struct Numpad {
      int fixedDigits = 0;
      std::optional<Digits> first;
      std::optional<Digits> middle;
      std::optional<Digits> last;
      std::optional<DeviceAction> finish;
      std::vector<UnknownElement> u; //todo do we have a start action?
    } numpad;
    Commands irCommands;

    uint32_t id;
    std::vector<UnknownElement> unknownProperties;
};

}
}
}
