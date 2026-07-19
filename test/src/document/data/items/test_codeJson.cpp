// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::data::serialiser (de)serialization of item::Blob,
 * binary::Block / binary::TimingStream, and binary::irProto::Code / Section.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "codeJson.h"

using namespace document::data;
using namespace binary::irProto;



// ---------------------------------------------------------------------------
// binary::Block / binary::TimingStream
// ---------------------------------------------------------------------------

TEST(BlockJson, RoundTripsMarkAndPause)
{
  binary::Block block = binary::Block::fromMarkPause(500, 1500);

  serialiser::ordered_json j;
  serialiser::toJson(j, block);

  EXPECT_EQ(j["Mark"], 500);
  EXPECT_EQ(j["Pause"], 1500);

  binary::Block block2 = binary::Block::fromMarkPause(0, 0);
  serialiser::fromJson(j, block2);

  EXPECT_EQ(block2.mark_us, 500);
  EXPECT_EQ(block2.pause_us, 1500);
}

TEST(BlockJson, SegmentIsDerivedNotStored)
{
  binary::Block block = binary::Block::fromMarkPause(500, 1500);

  serialiser::ordered_json j;
  serialiser::toJson(j, block);

  EXPECT_FALSE(j.contains("Segment"));
}

TEST(TimingStreamJson, RoundTripsMultipleBlocks)
{
  binary::TimingStream stream = binary::TimingStream::fromMarkPause({500, 1500, 300, 700});

  serialiser::ordered_json j;
  serialiser::toJson(j, stream);

  ASSERT_TRUE(j.is_array());
  EXPECT_EQ(j.size(), 2u);

  binary::TimingStream stream2;
  serialiser::fromJson(j, stream2);

  ASSERT_EQ(stream2.timings().size(), 2u);
  EXPECT_EQ(stream2.timings()[0].mark_us, 500);
  EXPECT_EQ(stream2.timings()[0].pause_us, 1500);
  EXPECT_EQ(stream2.timings()[1].mark_us, 300);
  EXPECT_EQ(stream2.timings()[1].pause_us, 700);
}

// ---------------------------------------------------------------------------
// binary::irProto::Section
// ---------------------------------------------------------------------------

TEST(SectionJson, RoundTrips)
{
  Section section(2, {true, false, true, true});

  serialiser::ordered_json j;
  serialiser::toJson(j, section);

  EXPECT_EQ(j["Index"], 2);

  Section section2(0, {});
  serialiser::fromJson(j, section2);

  EXPECT_EQ(section2.getIndex(), 2);
  EXPECT_EQ(section2.getData(), (std::vector<bool>{true, false, true, true}));
}

// ---------------------------------------------------------------------------
// binary::irProto::Code
// ---------------------------------------------------------------------------

TEST(CodeJson, RawFieldsRoundTrip)
{
  Code code;
  code.setIndex(7);
  code.setTicks(526);
  code.setDataSectionCount(2);
  code.setRepeatFrame(1);
  code.setDataFrameTxCount(1);
  code.setControl(3);

  serialiser::ordered_json j;
  serialiser::toJson(j, code);

  EXPECT_EQ(j["Index"], 7);
  EXPECT_EQ(j["Ticks"], 526);
  EXPECT_EQ(j["DataSectionCount"], 2);
  EXPECT_EQ(j["RepeatFrame"], 1);
  EXPECT_EQ(j["Control"], 3);

  Code code2;
  serialiser::fromJson(j, code2);

  EXPECT_EQ(code2.getIndex(), 7);
  EXPECT_EQ(code2.getTicks(), 526);
  EXPECT_EQ(code2.getDataSectionCount(), 2);
  EXPECT_EQ(code2.getRepeatFrame(), 1);
  EXPECT_EQ(code2.getControl(), 3);
}

TEST(CodeJson, SectionsRoundTrip)
{
  Code code;
  code.setSections({Section(0, {true, false}), Section(1, {})});

  serialiser::ordered_json j;
  serialiser::toJson(j, code);

  ASSERT_TRUE(j["Sections"].is_array());
  EXPECT_EQ(j["Sections"].size(), 2u);

  Code code2;
  serialiser::fromJson(j, code2);

  ASSERT_EQ(code2.accessSections().size(), 2u);
  EXPECT_EQ(code2.accessSections()[0].getIndex(), 0);
  EXPECT_EQ(code2.accessSections()[0].getData(), (std::vector<bool>{true, false}));
  EXPECT_EQ(code2.accessSections()[1].getIndex(), 1);
}

TEST(CodeJson, FullRoundTripProducesIdenticalJson)
{
  Code code;
  code.setIndex(3);
  code.setTicks(526);
  code.setDataSectionCount(1);
  code.setRepeatFrame(0);
  code.setDataFrameTxCount(2);
  code.setControl(0);
  code.setSections({Section(0, {true, true, false})});

  serialiser::ordered_json j;
  serialiser::toJson(j, code);

  Code code2;
  serialiser::fromJson(j, code2);

  serialiser::ordered_json j2;
  serialiser::toJson(j2, code2);

  EXPECT_EQ(j, j2);
}
