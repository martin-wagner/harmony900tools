// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::data::serialiser (de)serialization of
 * binary::irProto::File and its dependencies (Item, TimingSectionIrHeader,
 * TimingSectionIrPayload, TimingSection, IrProto).
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "irProtoJson.h"

using namespace document::data;
using namespace binary::irProto;

// ---------------------------------------------------------------------------
// Item (std::pair<bool,uint16_t>)
// ---------------------------------------------------------------------------

TEST(IrProtoItemJson, RoundTrips)
{
  Item item{true, 500};

  serialiser::ordered_json j;
  serialiser::toJson(j, item);

  EXPECT_EQ(j["State"], true);
  EXPECT_EQ(j["Time"], 500);

  Item item2{false, 0};
  serialiser::fromJson(j, item2);

  EXPECT_EQ(item2.first, true);
  EXPECT_EQ(item2.second, 500);
}

// ---------------------------------------------------------------------------
// TimingSectionIrHeader
// ---------------------------------------------------------------------------

TEST(TimingSectionIrHeaderJson, RoundTrips)
{
  TimingSectionIrHeader header;
  header.addItem({true, 500});
  header.addItem({false, 1500});

  serialiser::ordered_json j;
  serialiser::toJson(j, header);

  ASSERT_TRUE(j.is_array());
  EXPECT_EQ(j.size(), 2u);

  TimingSectionIrHeader header2;
  serialiser::fromJson(j, header2);

  ASSERT_EQ(header2.getCount(), 2);
  EXPECT_EQ(header2.accessStream()[0].first, true);
  EXPECT_EQ(header2.accessStream()[0].second, 500);
  EXPECT_EQ(header2.accessStream()[1].first, false);
  EXPECT_EQ(header2.accessStream()[1].second, 1500);
}

TEST(TimingSectionIrHeaderJson, EmptyHeaderRoundTrips)
{
  TimingSectionIrHeader header;

  serialiser::ordered_json j;
  serialiser::toJson(j, header);

  TimingSectionIrHeader header2;
  serialiser::fromJson(j, header2);

  EXPECT_TRUE(header2.isEmpty());
}

// ---------------------------------------------------------------------------
// TimingSectionIrPayload
// ---------------------------------------------------------------------------

TEST(TimingSectionIrPayloadJson, RoundTripsBothPairs)
{
  TimingSectionIrPayload payload;
  payload.setFalsePair({{true, 300}, {false, 900}});
  payload.setTruePair({{true, 600}, {false, 600}});

  serialiser::ordered_json j;
  serialiser::toJson(j, payload);

  TimingSectionIrPayload payload2;
  serialiser::fromJson(j, payload2);

  ASSERT_EQ(payload2.accessFalsePair().size(), 2u);
  EXPECT_EQ(payload2.accessFalsePair()[0].second, 300);
  ASSERT_EQ(payload2.accessTruePair().size(), 2u);
  EXPECT_EQ(payload2.accessTruePair()[0].second, 600);
  EXPECT_TRUE(payload2.haveFalse());
  EXPECT_TRUE(payload2.haveTrue());
}

// ---------------------------------------------------------------------------
// TimingSection
// ---------------------------------------------------------------------------

TEST(TimingSectionJson, EnumFieldsRoundTripAsStrings)
{
  TimingSection section;
  section.setCtrl0(TimingSection::Ctrl0::IS_REPEAT_FRAME);
  section.setCtrl1(TimingSection::Ctrl1::DATA_ONE_PAIR);

  serialiser::ordered_json j;
  serialiser::toJson(j, section);

  EXPECT_EQ(j["Ctrl0"], "IsRepeatFrame");
  EXPECT_EQ(j["Ctrl1"], "DataOnePair");

  TimingSection section2;
  serialiser::fromJson(j, section2);

  EXPECT_EQ(section2.getCtrl0(), TimingSection::Ctrl0::IS_REPEAT_FRAME);
  EXPECT_EQ(section2.getCtrl1(), TimingSection::Ctrl1::DATA_ONE_PAIR);
}

TEST(TimingSectionJson, ScalarFieldsRoundTrip)
{
  TimingSection section;
  section.setBitCount(16);
  section.setToggle(3);
  section.setTiming(12000);

  serialiser::ordered_json j;
  serialiser::toJson(j, section);

  EXPECT_EQ(j["BitCount"], 16);
  EXPECT_EQ(j["Toggle"], 3);
  EXPECT_EQ(j["Timing"], 12000);

  TimingSection section2;
  serialiser::fromJson(j, section2);

  EXPECT_EQ(section2.getBitCount(), 16);
  EXPECT_EQ(section2.getToggle(), 3);
  EXPECT_TRUE(section2.hasToggle());
  EXPECT_EQ(section2.getTiming(), 12000u);
}

TEST(TimingSectionJson, NestedSoFDataEoFRoundTrip)
{
  TimingSection section;

  TimingSectionIrHeader sof;
  sof.addItem({true, 100});
  section.setSoF(sof);

  TimingSectionIrPayload data;
  data.setTruePair({{true, 50}, {false, 50}});
  section.setData(data);

  TimingSectionIrHeader eof;
  eof.addItem({false, 200});
  section.setEoF(eof);

  serialiser::ordered_json j;
  serialiser::toJson(j, section);

  TimingSection section2;
  serialiser::fromJson(j, section2);

  ASSERT_EQ(section2.getSoF().getCount(), 1);
  EXPECT_EQ(section2.getSoF().accessStream()[0].second, 100);

  EXPECT_TRUE(section2.getData().haveTrue());
  ASSERT_EQ(section2.getData().accessTruePair().size(), 2u);

  ASSERT_EQ(section2.getEoF().getCount(), 1);
  EXPECT_EQ(section2.getEoF().accessStream()[0].second, 200);
}

// ---------------------------------------------------------------------------
// IrProto
// ---------------------------------------------------------------------------

TEST(IrProtoJson, ClockPeriodRoundTrips)
{
  IrProto proto(26316, {});

  serialiser::ordered_json j;
  serialiser::toJson(j, proto, "Test");

  EXPECT_EQ(j["ClockPeriod"], 26316);

  IrProto proto2;
  serialiser::fromJson(j, proto2);

  EXPECT_EQ(proto2.getClockPeriod(), 26316);
}

TEST(IrProtoJson, SectionsRoundTrip)
{
  TimingSection section;
  section.setBitCount(8);
  IrProto proto(1234, {section});

  serialiser::ordered_json j;
  serialiser::toJson(j, proto, "Test");

  ASSERT_TRUE(j["Sections"].is_array());
  EXPECT_EQ(j["Sections"].size(), 1u);

  IrProto proto2;
  serialiser::fromJson(j, proto2);

  ASSERT_EQ(proto2.getSectionCount(), 1);
  EXPECT_EQ(proto2.accessSection(0).getBitCount(), 8);
}

