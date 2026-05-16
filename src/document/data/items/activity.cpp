// SPDX-License-Identifier: LGPL-2.1-or-later

#include "activity.h"

using namespace std;

namespace document
{
namespace data
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
    getPvrType().setIncluded(Include::CHECK);
    getControlGroup_HardButtons().setIncluded(Include::CHECK);
    getPowerOffUnusedDevices().setIncluded(Include::CHECK);
    getTrainingWheels().setIncluded(Include::CHECK);
    getUnusedDevicesHelp().setIncluded(Include::CHECK);
  }
}

}
}
}
