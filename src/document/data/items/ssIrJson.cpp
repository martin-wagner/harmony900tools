// SPDX-License-Identifier: LGPL-2.1-or-later

#include "ssIrJson.h"
#include "codeJson.h" //TimingStream (de)serialization

using namespace std;

namespace document
{
namespace data
{
namespace serialiser
{

void toJson(ordered_json &out, const binary::ssIr::SerialStreamIr &stream)
{
  out["ClockPeriod"] = stream.getClockPeriod();

  ordered_json timingsJson;
  toJson(timingsJson, stream.accessStream());
  out["Timings"] = timingsJson;
}

void fromJson(const ordered_json &in, binary::ssIr::SerialStreamIr &stream)
{
  binary::TimingStream timingStream;
  auto timingsIt = in.find("Timings");
  if (timingsIt != in.end()) {
    fromJson(*timingsIt, timingStream);
  }

  stream = binary::ssIr::SerialStreamIr(timingStream,
      binary::ssIr::DEFAULT_CLOCK_HZ);
  stream.setClockPeriod(in.value("ClockPeriod", uint16_t(26316)));
}

}
}
}
