// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::data::serialiser (de)serialization of
 * binary::ssIr::File and binary::ssIr::SerialStreamIr.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "ssIrJson.h"

using namespace document::data;
using namespace binary::ssIr;

// ---------------------------------------------------------------------------
// SerialStreamIr
// ---------------------------------------------------------------------------

TEST(SerialStreamIrJson, ClockPeriodRoundTripsExactly)
{
  binary::TimingStream ts = binary::TimingStream::fromMarkPause({500, 1500});
  SerialStreamIr stream(ts, DEFAULT_CLOCK_HZ);
  stream.setClockPeriod(12345); // exact raw value, not derivable losslessly from Hz

  serialiser::ordered_json j;
  serialiser::toJson(j, stream);

  EXPECT_EQ(j["ClockPeriod"], 12345);

  SerialStreamIr stream2;
  serialiser::fromJson(j, stream2);

  EXPECT_EQ(stream2.getClockPeriod(), 12345);
}

TEST(SerialStreamIrJson, TimingsRoundTrip)
{
  binary::TimingStream ts = binary::TimingStream::fromMarkPause({500, 1500, 300, 700});
  SerialStreamIr stream(ts, DEFAULT_CLOCK_HZ);

  serialiser::ordered_json j;
  serialiser::toJson(j, stream);

  ASSERT_TRUE(j["Timings"].is_array());
  EXPECT_EQ(j["Timings"].size(), 2u);

  SerialStreamIr stream2;
  serialiser::fromJson(j, stream2);

  ASSERT_EQ(stream2.accessStream().timings().size(), 2u);
  EXPECT_EQ(stream2.accessStream().timings()[0].mark_us, 500);
  EXPECT_EQ(stream2.accessStream().timings()[0].pause_us, 1500);
}
