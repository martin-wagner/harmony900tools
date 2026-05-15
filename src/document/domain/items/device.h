// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>

#include "document/domain/enum.h"
#include "document/domain/property.h"
#include "unknown.h"
#include "state.h"
#include "digits.h"
#include "commands.h"
#include "presentation.h"

namespace document
{
namespace domain
{
namespace item
{

/** stores one single device info struct from UserConfiguration.xml
 */
class Device
{
  public:
    Device(uint32_t id) : id(id) {};

    uint32_t getId() const
    {
      return id;
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
      Property<bool> alwaysOn{false, Include::OPTIONAL};
      Property<bool> autoPower{false, Include::OPTIONAL};
      //other
      Property<bool> audioSwitch{false, Include::OPTIONAL};
      Property<bool> dimmer{false, Include::OPTIONAL};
      Property<bool> hasBands{true, Include::OPTIONAL};
      Property<bool> hasPresets{true, Include::OPTIONAL};
      Property<bool> isNewDevice{true, Include::OPTIONAL};
      Property<bool> isDisplayDevice{true, Include::OPTIONAL};
      Property<bool> menuOnDevice{false, Include::OPTIONAL};
      Property<int> numDiscs{1, Include::OPTIONAL};
      Property<int> numLights{1, Include::OPTIONAL};
      Property<bool> onScreenGuide{false, Include::OPTIONAL};
      Property<Enum<PvrType>> pvrType{PvrType::Generic, Include::OPTIONAL};
      Property<bool> recordMediaFixedDisc{true, Include::OPTIONAL};
      Property<bool> recordMediaRemovableVideotape{true, Include::OPTIONAL};
      Property<bool> revertInput{true, Include::OPTIONAL};
      Property<bool> scart{true, Include::OPTIONAL};
      Property<Enum<TunerInput>> tunerInput{TunerInput::Tuner, Include::OPTIONAL};
      Property<bool> videoSwitch{true, Include::OPTIONAL};
      //other other
      std::vector<UnknownElement> u;
    } p;

    const uint32_t id;
    Enum<DeviceType> type{DeviceType::Computer};
    std::string manufacturer = "Unknown";
    std::string model = "Unknown";
    Presentation buttons;
    std::vector<StateMachine> stateMachines;
    struct Numpad {
      int fixedDigits = 0;
      std::unique_ptr<Digits> first;
      std::unique_ptr<Digits> middle;
      std::unique_ptr<Digits> last;
      std::unique_ptr<DeviceAction> finish;
      std::vector<UnknownElement> u; //todo do we have a start action?
    } numpad;
    Commands irCommands;

    std::vector<UnknownElement> u;
};

}
}
}
