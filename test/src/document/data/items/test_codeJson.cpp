// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::data::serialiser (de)serialization of
 * binary::Block / binary::TimingStream and binary::irProto::Group / Code,
 * including the pre v0.2 legacy Code format.
 */

#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

#include "bin/irProto/dummyProto.h"
#include "codeJson.h"

using namespace document::data;
using namespace binary::irProto;

std::string toHexString(uint64_t value)
{
  std::stringstream stream;
  stream << "0x" << std::hex << value;
  return stream.str();
}

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
  binary::TimingStream stream = binary::TimingStream::fromMarkPause( {
    500,
    1500,
    300,
    700 });

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
// binary::irProto::Group
// ---------------------------------------------------------------------------

TEST(GroupJson, CompactBitsRoundTripUsesHexString)
{
  Group group(1, 1, Symbols { 1, 0, 1, 1, 0, 0, 1, 0 });
  Group group2;

  serialiser::ordered_json j;
  serialiser::toJson(j, group);

  EXPECT_EQ(j["Section"], 1);
  EXPECT_EQ(j["Bits"], 1);
  EXPECT_EQ(j["DataQuantumCount"], 8);
  ASSERT_TRUE(j["Data"].is_string());
  EXPECT_EQ(j["Data"].get<std::string>(),
      toHexString(Code::listToU64bits(group.getData())));

  serialiser::fromJson(j, group2);

  EXPECT_EQ(group2.getSection(), 1);
  EXPECT_EQ(group2.getBits(), 1);
  EXPECT_EQ(group2.getData(), group.getData());
}

TEST(GroupJson, MultiBitSymbolsRoundTripUsesArray)
{
  Group group(2, 2, Symbols { 3, 1, 2, 0 });
  Group group2;

  serialiser::ordered_json j;
  serialiser::toJson(j, group);

  EXPECT_EQ(j["Section"], 2);
  EXPECT_EQ(j["Bits"], 2);
  EXPECT_EQ(j["DataQuantumCount"], 4);
  ASSERT_TRUE(j["Data"].is_array());
  EXPECT_EQ(j["Data"].get<Symbols>(), (Symbols { 3, 1, 2, 0 }));

  serialiser::fromJson(j, group2);

  EXPECT_EQ(group2.getSection(), 2);
  EXPECT_EQ(group2.getBits(), 2);
  EXPECT_EQ(group2.getData(), (Symbols { 3, 1, 2, 0 }));
}

TEST(GroupJson, EmptyGroupRoundTrips)
{
  Group group(1, 2, Symbols { });
  Group group2;

  serialiser::ordered_json j;
  serialiser::toJson(j, group);

  serialiser::fromJson(j, group2);

  EXPECT_EQ(group2.getSection(), 1);
  EXPECT_EQ(group2.getBits(), 2);
  EXPECT_TRUE(group2.getData().empty());
}

TEST(GroupJson, ParsesHexStringWithBitCount)
{
  Group group;
  serialiser::ordered_json j =
      serialiser::ordered_json::parse(
          R"({
    "Section": 0,
    "Bits": 2,
    "DataQuantumCount": 32,
    "Data": "0xb4b4fe01"
  })");

  serialiser::fromJson(j, group);

  EXPECT_EQ(group.getSection(), 0);
  EXPECT_EQ(group.getBits(), 2);
  EXPECT_EQ(group.getData(), Code::u64bitsToList(32, 0xb4b4fe01));
}

TEST(GroupJson, MissingFieldsUseDefaults)
{
  Group group;
  serialiser::ordered_json j = serialiser::ordered_json::object();

  serialiser::fromJson(j, group);

  EXPECT_EQ(group.getSection(), 0);
  EXPECT_EQ(group.getBits(), 1);
  EXPECT_TRUE(group.getData().empty());
}

// ---------------------------------------------------------------------------
// binary::irProto::Code (current format)
// ---------------------------------------------------------------------------

TEST(CodeJson, RawFieldsRoundTrip)
{
  Code code(7, protoNec());
  code.setDelay(526);
  code.setRepeats(2);

  serialiser::ordered_json j;
  serialiser::toJson(j, code);

  EXPECT_EQ(j["Delay"], 526);
  EXPECT_EQ(j["Repeats"], 2);

  Code code2(7, protoNec());
  serialiser::fromJson(j, code2);

  EXPECT_EQ(code2.getDelay(), 526);
  EXPECT_EQ(code2.getRepeats(), 2);
}

TEST(CodeJson, GroupsRoundTrip)
{
  Code code(1, protoNec());
  code.accessStartGroups().push_back(Group(0, 2, Symbols { 1, 0, 1, 1 }));
  code.accessRepeatGroups().push_back(Group(1, 2, Symbols { }));
  code.accessFinishGroups().push_back(Group(0, 4, Symbols { 3, 2, 1 }));

  serialiser::ordered_json j;
  serialiser::toJson(j, code);

  ASSERT_TRUE(j["StartGroups"].is_array());
  ASSERT_TRUE(j["RepeatGroups"].is_array());
  ASSERT_TRUE(j["FinishGroups"].is_array());
  EXPECT_EQ(j["StartGroups"].size(), 1u);
  EXPECT_EQ(j["RepeatGroups"].size(), 1u);
  EXPECT_EQ(j["FinishGroups"].size(), 1u);

  Code code2(1, protoNec());
  serialiser::fromJson(j, code2);

  ASSERT_EQ(code2.accessStartGroups().size(), 1u);
  EXPECT_EQ(code2.accessStartGroups()[0].getSection(), 0);
  EXPECT_EQ(code2.accessStartGroups()[0].getBits(), 2);
  EXPECT_EQ(code2.accessStartGroups()[0].getData(), (Symbols { 1, 0, 1, 1 }));

  ASSERT_EQ(code2.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(code2.accessRepeatGroups()[0].getSection(), 1);
  EXPECT_TRUE(code2.accessRepeatGroups()[0].getData().empty());

  ASSERT_EQ(code2.accessFinishGroups().size(), 1u);
  EXPECT_EQ(code2.accessFinishGroups()[0].getSection(), 0);
  EXPECT_EQ(code2.accessFinishGroups()[0].getBits(), 4);
  EXPECT_EQ(code2.accessFinishGroups()[0].getData(), (Symbols { 3, 2, 1 }));
}

TEST(CodeJson, FromJsonReplacesExistingGroups)
{
  Code source(1, protoNec());
  source.accessStartGroups().push_back(Group(0, 2, Symbols { 1, 0 }));

  serialiser::ordered_json j;
  serialiser::toJson(j, source);

  Code target(1, protoNec());
  target.accessStartGroups().push_back(Group(0, 2, Symbols { 0, 0, 0 }));
  target.accessStartGroups().push_back(Group(0, 2, Symbols { 1, 1, 1 }));
  target.accessFinishGroups().push_back(Group(0, 2, Symbols { 1 }));
  serialiser::fromJson(j, target);

  ASSERT_EQ(target.accessStartGroups().size(), 1u);
  EXPECT_EQ(target.accessStartGroups()[0].getData(), (Symbols { 1, 0 }));
  EXPECT_TRUE(target.accessFinishGroups().empty());
}

TEST(CodeJson, MissingFieldsUseDefaults)
{
  serialiser::ordered_json j = serialiser::ordered_json::object();

  Code code(1, protoNec());
  serialiser::fromJson(j, code);

  EXPECT_EQ(code.getDelay(), Code::DEFAULT_PRE_SEND_SILENCE);
  EXPECT_EQ(code.getRepeats(), Code::DEFAULT_REPEATS);
  EXPECT_TRUE(code.accessStartGroups().empty());
  EXPECT_TRUE(code.accessRepeatGroups().empty());
  EXPECT_TRUE(code.accessFinishGroups().empty());
}

TEST(CodeJson, FullRoundTripProducesIdenticalJson)
{
  Code code(3, protoNec());
  code.setDelay(526);
  code.setRepeats(2);
  code.accessStartGroups().push_back(Group(0, 2, Symbols { 1, 1, 0, 1 }));
  code.accessRepeatGroups().push_back(Group(1, 2, Symbols { }));
  code.accessFinishGroups().push_back(Group(0, 4, Symbols { 2, 3 }));

  serialiser::ordered_json j;
  serialiser::toJson(j, code);

  Code code2(3, protoNec());
  serialiser::fromJson(j, code2);

  serialiser::ordered_json j2;
  serialiser::toJson(j2, code2);

  EXPECT_EQ(j, j2);
}

// ---------------------------------------------------------------------------
// binary::irProto::Code (legacy format, pre v0.2)
// ---------------------------------------------------------------------------

TEST(CodeJsonLegacy, ControlZeroBecomesStartGroupsWithEmptyRepeat)
{
  serialiser::ordered_json j =
      serialiser::ordered_json::parse(
          R"({
    "Index": 1,
    "Delay": 500,
    "RepeatFrame": 1,
    "DataFrameTxCount": 1,
    "Control": 0,
    "Sections": [
      {
        "Index": 0,
        "Data": "0xb4b4fe01",
        "DataBitCount": 32
      }
    ]
  })");

  Code code(1, protoNec());
  serialiser::fromJson(j, code);

  EXPECT_EQ(code.getIndex(), 1);
  EXPECT_EQ(code.getDelay(), 500);
  EXPECT_EQ(code.getRepeats(), 1);

  ASSERT_EQ(code.accessStartGroups().size(), 1u);
  EXPECT_EQ(code.accessStartGroups()[0].getSection(), 0);
  EXPECT_EQ(code.accessStartGroups()[0].getBits(), 1);
  EXPECT_EQ(code.accessStartGroups()[0].getData(),
      Code::u64bitsToList(32, 0xb4b4fe01));

  ASSERT_EQ(code.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(code.accessRepeatGroups()[0].getSection(), 1);
  EXPECT_TRUE(code.accessRepeatGroups()[0].getData().empty());

  EXPECT_TRUE(code.accessFinishGroups().empty());
}

TEST(CodeJsonLegacy, ControlOneBecomesRepeatGroups)
{
  serialiser::ordered_json j =
      serialiser::ordered_json::parse(
          R"({
    "Index": 3,
    "Delay": 500,
    "RepeatFrame": 0,
    "DataFrameTxCount": 0,
    "Control": 1,
    "Sections": [
      {
        "Index": 0,
        "Data": "0xffc1ef8c",
        "DataBitCount": 32
      }
    ]
  })");

  Code code(3, protoSingle(32));
  serialiser::fromJson(j, code);

  EXPECT_EQ(code.getIndex(), 3);
  EXPECT_EQ(code.getDelay(), 500);
  EXPECT_EQ(code.getRepeats(), 0);

  EXPECT_TRUE(code.accessStartGroups().empty());

  ASSERT_EQ(code.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(code.accessRepeatGroups()[0].getSection(), 0);
  EXPECT_EQ(code.accessRepeatGroups()[0].getBits(), 1);
  EXPECT_EQ(code.accessRepeatGroups()[0].getData(),
      Code::u64bitsToList(32, 0xffc1ef8c));

  EXPECT_TRUE(code.accessFinishGroups().empty());
}

TEST(CodeJsonLegacy, SectionIndexIsPositionInList)
{
  serialiser::ordered_json j =
      serialiser::ordered_json::parse(
          R"({
    "Index": 3,
    "Delay": 500,
    "DataFrameTxCount": 2,
    "Control": 3,
    "Sections": [
      { "Index": 0, "Data": "0x8001", "DataBitCount": 16 },
      { "Index": 1, "Data": "0x2", "DataBitCount": 16 },
      { "Index": 2, "Data": "0xabcd", "DataBitCount": 16 }
    ]
  })");

  Code code(3, protoSingle(32));
  serialiser::fromJson(j, code);

  EXPECT_EQ(code.getRepeats(), 2);
  EXPECT_TRUE(code.accessStartGroups().empty());

  ASSERT_EQ(code.accessRepeatGroups().size(), 3u);
  EXPECT_EQ(code.accessRepeatGroups()[0].getSection(), 0);
  EXPECT_EQ(code.accessRepeatGroups()[1].getSection(), 1);
  EXPECT_EQ(code.accessRepeatGroups()[2].getSection(), 2);
  //hard-coded fix for the header
  EXPECT_EQ(code.accessRepeatGroups()[0].getData(),
      Code::u64bitsToList(4, 0x8));
  EXPECT_EQ(code.accessRepeatGroups()[1].getData(),
      Code::u64bitsToList(1, 0x0));
  EXPECT_EQ(code.accessRepeatGroups()[2].getData(),
      Code::u64bitsToList(16, 0xabcd));
}
