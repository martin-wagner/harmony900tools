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
#include "presentation.h"

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
    std::set<uint32_t> getAllIds() {}; //todo

    const Presentation& getButtons() const
    {
      return buttons;
    }

    void setButtons(const Presentation &buttons)
    {
      this->buttons = buttons;
    }

    const Commands& getIrCommands() const
    {
      return irCommands;
    }

    void setIrCommands(const Commands &irCommands)
    {
      this->irCommands = irCommands;
    }

    const std::string& getManufacturer() const
    {
      return manufacturer;
    }

    void setManufacturer(const std::string &manufacturer = "Unknown")
    {
      this->manufacturer = manufacturer;
    }

    const std::string& getModel() const
    {
      return model;
    }

    void setModel(const std::string &model = "Unknown")
    {
      this->model = model;
    }

    const std::vector<StateMachine>& getStateMachines() const
    {
      return stateMachines;
    }

    void setStateMachines(const std::vector<StateMachine> &stateMachines)
    {
      this->stateMachines = stateMachines;
    }

    const Enum<DeviceType>& getType() const
    {
      return type;
    }

    void setType(const Enum<DeviceType>& type)
    {
      this->type = type;
    }

    const std::vector<UnknownElement>& getU() const
    {
      return u;
    }

    void setU(const std::vector<UnknownElement> &u)
    {
      this->u = u;
    }

    PROPERTY_GETTER(Property<bool>, manualPower, getManualPower)
    PROPERTY_GETTER(Property<bool>, alwaysOn, getAlwaysOn)
    PROPERTY_GETTER(Property<bool>, autoPower, getAutoPower)
    PROPERTY_GETTER(Property<bool>, audioSwitch, getAudioSwitch)
    PROPERTY_GETTER(Property<bool>, dimmer, getDimmer)
    PROPERTY_GETTER(Property<bool>, hasBands, getHasBands)
    PROPERTY_GETTER(Property<bool>, hasPresets, getHasPresets)
    PROPERTY_GETTER(Property<bool>, isNewDevice, getIsNewDevice)
    PROPERTY_GETTER(Property<bool>, isDisplayDevice, getIsDisplayDevice)
    PROPERTY_GETTER(Property<bool>, menuOnDevice, getMenuOnDevice)
    PROPERTY_GETTER(Property<int>, numDiscs, getNumDiscs)
    PROPERTY_GETTER(Property<int>, numLights, getNumLights)
    PROPERTY_GETTER(Property<bool>, onScreenGuide, getOnScreenGuide)
    PROPERTY_GETTER(Property<Enum<PvrType>>, pvrType, getPvrType)
    PROPERTY_GETTER(Property<bool>, recordMediaFixedDisc, getRecordMediaFixedDisc)
    PROPERTY_GETTER(Property<bool>, recordMediaRemovableVideotape, getRecordMediaRemovableVideotape)
    PROPERTY_GETTER(Property<bool>, revertInput, getRevertInput)
    PROPERTY_GETTER(Property<bool>, scart, getScart)
    PROPERTY_GETTER(Property<Enum<TunerInput>>, tunerInput, getTunerInput)
    PROPERTY_GETTER(Property<bool>, videoSwitch, getVideoSwitch)

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
      //power related
      Property<bool> manualPower{false, Include::ALWAYS};
      Property<bool> alwaysOn{false, Include::CHECK};
      Property<bool> autoPower{false, Include::CHECK};
      //other
      Property<bool> audioSwitch{false, Include::CHECK};
      Property<bool> dimmer{false, Include::CHECK};
      Property<bool> hasBands{true, Include::CHECK};
      Property<bool> hasPresets{true, Include::CHECK};
      Property<bool> isNewDevice{true, Include::CHECK};
      Property<bool> isDisplayDevice{true, Include::CHECK};
      Property<bool> menuOnDevice{false, Include::CHECK};
      Property<int> numDiscs{1, Include::CHECK};
      Property<int> numLights{1, Include::CHECK};
      Property<bool> onScreenGuide{false, Include::CHECK};
      Property<Enum<PvrType>> pvrType{PvrType::Generic, Include::CHECK};
      Property<bool> recordMediaFixedDisc{true, Include::CHECK};
      Property<bool> recordMediaRemovableVideotape{true, Include::CHECK};
      Property<bool> revertInput{true, Include::CHECK};
      Property<bool> scart{true, Include::CHECK};
      Property<Enum<TunerInput>> tunerInput{TunerInput::Tuner, Include::CHECK};
      Property<bool> videoSwitch{true, Include::CHECK};
      //other other
      std::vector<UnknownElement> u;
    } p;

    uint32_t id;
    Enum<DeviceType> type{DeviceType::Computer};
    std::string manufacturer = "Unknown";
    std::string model = "Unknown";
    Presentation buttons;
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

    std::vector<UnknownElement> u;
};

}
}
}
