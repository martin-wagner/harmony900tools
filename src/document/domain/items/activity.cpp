// SPDX-License-Identifier: LGPL-2.1-or-later

#include "activity.h"

using namespace std;

namespace document
{
namespace domain
{
namespace item
{

Activity::Activity(uint32_t id, bool powerOff) :
    powerOff(powerOff), id(id)
{
  if (powerOff) {
    id = -1;
    type = Enum<ActivityType>(ActivityType::PowerOff);
    //no properties
    getPvrType().setIncluded(Include::OPTIONAL);
    getControlGroup_HardButtons().setIncluded(Include::OPTIONAL);
    getPowerOffUnusedDevices().setIncluded(Include::OPTIONAL);
    getTrainingWheels().setIncluded(Include::OPTIONAL);
    getUnusedDevicesHelp().setIncluded(Include::OPTIONAL);
  }
}

}
}
}
