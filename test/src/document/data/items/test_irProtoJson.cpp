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

TEST(TimingSectionIrPayloadJson, BooleanQuantaRoundTrip)
{
  TimingSectionIrPayload::Quantum falseQuantum = {{true, 300}, {false, 900}};
  TimingSectionIrPayload::Quantum trueQuantum = {{true, 600}, {false, 600}};

  TimingSectionIrPayload payload;
  payload.setQuantum(0, falseQuantum);
  payload.setQuantum(1, trueQuantum);

  serialiser::ordered_json j;
  serialiser::toJson(j, payload);

  ASSERT_TRUE(j["Quanta"].is_array());
  EXPECT_EQ(j["Quanta"].size(), 2u);

  TimingSectionIrPayload payload2;
  serialiser::fromJson(j, payload2);

  EXPECT_TRUE(payload2.isBoolean());
  ASSERT_EQ(payload2.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE).size(), 2u);
  EXPECT_EQ(payload2.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[0].second, 300);
  EXPECT_EQ(payload2.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[1].second, 900);
  ASSERT_EQ(payload2.accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE).size(), 2u);
  EXPECT_EQ(payload2.accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE)[0].second, 600);
  EXPECT_EQ(payload2.accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE)[1].second, 600);
}

TEST(TimingSectionIrPayloadJson, QuadQuantaRoundTrip)
{
  TimingSectionIrPayload payload;
  for (uint8_t i = 0; i < 4; i++) {
    TimingSectionIrPayload::Quantum quantum = {{true, static_cast<uint16_t>(100 + i)}, {false, static_cast<uint16_t>(200 + i)}};
    payload.setQuantum(i, quantum);
  }

  serialiser::ordered_json j;
  serialiser::toJson(j, payload);

  EXPECT_EQ(j["Quanta"].size(), 4u);

  TimingSectionIrPayload payload2;
  serialiser::fromJson(j, payload2);

  ASSERT_TRUE(payload2.isQuad());
  for (uint8_t i = 0; i < 4; i++) {
    ASSERT_EQ(payload2.accessQuantum(i).size(), 2u);
    EXPECT_EQ(payload2.accessQuantum(i)[0].second, 100 + i);
    EXPECT_EQ(payload2.accessQuantum(i)[1].second, 200 + i);
  }
}

TEST(TimingSectionIrPayloadJson, EmptyPayloadRoundTrips)
{
  TimingSectionIrPayload payload;

  serialiser::ordered_json j;
  serialiser::toJson(j, payload);

  ASSERT_TRUE(j["Quanta"].is_array());
  EXPECT_TRUE(j["Quanta"].empty());

  TimingSectionIrPayload payload2;
  serialiser::fromJson(j, payload2);

  EXPECT_TRUE(payload2.isEmpty());
}

TEST(TimingSectionIrPayloadJson, MissingQuantaGivesEmptyPayload)
{
  serialiser::ordered_json j = serialiser::ordered_json::object();

  TimingSectionIrPayload payload;
  serialiser::fromJson(j, payload);

  EXPECT_TRUE(payload.isEmpty());
}

TEST(TimingSectionIrPayloadJson, LegacyFalseAndTruePair)
{
  serialiser::ordered_json j = serialiser::ordered_json::parse(R"({
    "FalsePair": [
      { "State": true, "Time": 568 },
      { "State": false, "Time": 552 }
    ],
    "TruePair": [
      { "State": true, "Time": 568 },
      { "State": false, "Time": 1662 }
    ]
  })");

  TimingSectionIrPayload payload;
  serialiser::fromJsonOld(j, payload);

  EXPECT_TRUE(payload.isBoolean());

  ASSERT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE).size(), 2u);
  EXPECT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[0].first, true);
  EXPECT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[0].second, 568);
  EXPECT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[1].first, false);
  EXPECT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[1].second, 552);

  ASSERT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE).size(), 2u);
  EXPECT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE)[0].second, 568);
  EXPECT_EQ(payload.accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE)[1].second, 1662);
}

TEST(TimingSectionIrPayloadJson, LegacyEmptyPairsGiveEmptyPayload)
{
  serialiser::ordered_json j = serialiser::ordered_json::parse(R"({
    "FalsePair": [],
    "TruePair": []
  })");

  TimingSectionIrPayload payload;
  serialiser::fromJsonOld(j, payload);

  EXPECT_TRUE(payload.isEmpty());
}

// ---------------------------------------------------------------------------
// TimingSection
// ---------------------------------------------------------------------------

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
  EXPECT_EQ(static_cast<int>(section2.getToggle()), 3);
  EXPECT_TRUE(section2.hasToggle());
  EXPECT_EQ(section2.getTiming(), 12000u);
}

TEST(TimingSectionJson, DefaultsRoundTrip)
{
  TimingSection section;

  serialiser::ordered_json j;
  serialiser::toJson(j, section);

  TimingSection section2;
  serialiser::fromJson(j, section2);

  EXPECT_EQ(section2.getBitCount(), 0);
  EXPECT_FALSE(section2.hasToggle());
  EXPECT_EQ(section2.getTiming(), TimingSection::PAUSE_IN_EOF);
  EXPECT_TRUE(section2.getSoF().isEmpty());
  EXPECT_TRUE(section2.getData().isEmpty());
  EXPECT_TRUE(section2.getEoF().isEmpty());
}

TEST(TimingSectionJson, MissingFieldsUseDefaults)
{
  serialiser::ordered_json j = serialiser::ordered_json::object();

  TimingSection section;
  serialiser::fromJson(j, section);

  EXPECT_EQ(section.getBitCount(), 0);
  EXPECT_FALSE(section.hasToggle());
  EXPECT_EQ(section.getTiming(), TimingSection::PAUSE_IN_EOF);
}

TEST(TimingSectionJson, NestedSoFDataEoFRoundTrip)
{
  TimingSection section;

  TimingSectionIrHeader sof;
  sof.addItem({true, 100});
  section.setSoF(sof);

  TimingSectionIrPayload::Quantum falseQuantum = {{true, 40}, {false, 40}};
  TimingSectionIrPayload::Quantum trueQuantum = {{true, 50}, {false, 50}};
  TimingSectionIrPayload data;
  data.setQuantum(0, falseQuantum);
  data.setQuantum(1, trueQuantum);
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

  EXPECT_TRUE(section2.getData().isBoolean());
  ASSERT_EQ(section2.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE).size(), 2u);
  EXPECT_EQ(section2.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[0].second, 40);
  ASSERT_EQ(section2.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE).size(), 2u);
  EXPECT_EQ(section2.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE)[0].second, 50);

  ASSERT_EQ(section2.getEoF().getCount(), 1);
  EXPECT_EQ(section2.getEoF().accessStream()[0].second, 200);
}

TEST(TimingSectionJson, LegacyDataFrame)
{
  serialiser::ordered_json j = serialiser::ordered_json::parse(R"({
    "BitCount": 32,
    "Toggle": 255,
    "Timing": 107870,
    "Ctrl0": "IsDataFrame",
    "Ctrl1": "DataTwoPairs",
    "SoF": [
      { "State": true, "Time": 8990 },
      { "State": false, "Time": 4490 }
    ],
    "Data": {
      "FalsePair": [
        { "State": true, "Time": 568 },
        { "State": false, "Time": 552 }
      ],
      "TruePair": [
        { "State": true, "Time": 568 },
        { "State": false, "Time": 1662 }
      ]
    },
    "EoF": [
      { "State": true, "Time": 568 }
    ]
  })");

  TimingSection section;
  serialiser::fromJson(j, section);

  EXPECT_EQ(section.getBitCount(), 32);
  EXPECT_FALSE(section.hasToggle());
  EXPECT_EQ(section.getTiming(), 107870u);

  ASSERT_EQ(section.getSoF().getCount(), 2);
  EXPECT_EQ(section.getSoF().accessStream()[0].first, true);
  EXPECT_EQ(section.getSoF().accessStream()[0].second, 8990);
  EXPECT_EQ(section.getSoF().accessStream()[1].first, false);
  EXPECT_EQ(section.getSoF().accessStream()[1].second, 4490);

  EXPECT_TRUE(section.getData().isBoolean());
  ASSERT_EQ(section.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE).size(), 2u);
  EXPECT_EQ(section.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_FALSE)[1].second, 552);
  ASSERT_EQ(section.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE).size(), 2u);
  EXPECT_EQ(section.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE)[1].second, 1662);

  ASSERT_EQ(section.getEoF().getCount(), 1);
  EXPECT_EQ(section.getEoF().accessStream()[0].first, true);
  EXPECT_EQ(section.getEoF().accessStream()[0].second, 568);
}

TEST(TimingSectionJson, LegacyRepeatFrame)
{
  serialiser::ordered_json j = serialiser::ordered_json::parse(R"({
    "BitCount": 0,
    "Toggle": 255,
    "Timing": 4294967295,
    "Ctrl0": "IsRepeatFrame",
    "Ctrl1": "NotDataFrame",
    "SoF": [
      { "State": true, "Time": 8990 },
      { "State": false, "Time": 2230 }
    ],
    "Data": {
      "FalsePair": [],
      "TruePair": []
    },
    "EoF": [
      { "State": true, "Time": 568 },
      { "State": false, "Time": 32767 },
      { "State": false, "Time": 32767 },
      { "State": false, "Time": 30543 }
    ]
  })");

  TimingSection section;
  serialiser::fromJson(j, section);

  EXPECT_EQ(section.getBitCount(), 0);
  EXPECT_FALSE(section.hasToggle());
  EXPECT_EQ(section.getTiming(), TimingSection::PAUSE_IN_EOF);

  ASSERT_EQ(section.getSoF().getCount(), 2);
  EXPECT_EQ(section.getSoF().accessStream()[1].second, 2230);

  EXPECT_TRUE(section.getData().isEmpty());

  ASSERT_EQ(section.getEoF().getCount(), 4);
  EXPECT_EQ(section.getEoF().accessStream()[0].second, 568);
  EXPECT_EQ(section.getEoF().accessStream()[1].second, 32767);
  EXPECT_EQ(section.getEoF().accessStream()[2].second, 32767);
  EXPECT_EQ(section.getEoF().accessStream()[3].second, 30543);
}

// ---------------------------------------------------------------------------
// IrProto
// ---------------------------------------------------------------------------

TEST(IrProtoJson, ClockPeriodRoundTrips)
{
  IrProto proto(26316, std::vector<TimingSection>());

  serialiser::ordered_json j;
  serialiser::toJson(j, proto, "Test");

  EXPECT_EQ(j["ClockPeriod"], 26316);

  IrProto proto2;
  serialiser::fromJson(j, proto2);

  EXPECT_EQ(proto2.getClockPeriod(), 26316);
}

TEST(IrProtoJson, NameRoundTrips)
{
  IrProto proto(26316, std::vector<TimingSection>());

  serialiser::ordered_json j;
  serialiser::toJson(j, proto, "MyProto");

  EXPECT_EQ(j["Name"], "MyProto");

  IrProto proto2;
  std::string name = serialiser::fromJson(j, proto2);

  EXPECT_EQ(name, "MyProto");
}

TEST(IrProtoJson, MissingClockPeriodUsesDefault)
{
  serialiser::ordered_json j = serialiser::ordered_json::object();

  IrProto proto;
  serialiser::fromJson(j, proto);

  EXPECT_EQ(proto.getClockPeriod(), 26316);
}

TEST(IrProtoJson, SectionsRoundTrip)
{
  TimingSection section;
  section.setBitCount(8);
  IrProto proto(1234, std::vector<TimingSection>{section});

  serialiser::ordered_json j;
  serialiser::toJson(j, proto, "Test");

  ASSERT_TRUE(j["Sections"].is_array());
  EXPECT_EQ(j["Sections"].size(), 1u);

  IrProto proto2;
  serialiser::fromJson(j, proto2);

  ASSERT_EQ(proto2.getSectionCount(), 1);
  EXPECT_EQ(proto2.accessSection(0).getBitCount(), 8);
}

TEST(IrProtoJson, LegacySections)
{
  serialiser::ordered_json j = serialiser::ordered_json::parse(R"({
    "Name": "Legacy",
    "ClockPeriod": 26316,
    "Sections": [
      {
        "BitCount": 32,
        "Toggle": 255,
        "Timing": 107870,
        "Ctrl0": "IsDataFrame",
        "Ctrl1": "DataTwoPairs",
        "SoF": [
          { "State": true, "Time": 8990 },
          { "State": false, "Time": 4490 }
        ],
        "Data": {
          "FalsePair": [
            { "State": true, "Time": 568 },
            { "State": false, "Time": 552 }
          ],
          "TruePair": [
            { "State": true, "Time": 568 },
            { "State": false, "Time": 1662 }
          ]
        },
        "EoF": [
          { "State": true, "Time": 568 }
        ]
      },
      {
        "BitCount": 0,
        "Toggle": 255,
        "Timing": 4294967295,
        "Ctrl0": "IsRepeatFrame",
        "Ctrl1": "NotDataFrame",
        "SoF": [
          { "State": true, "Time": 8990 },
          { "State": false, "Time": 2230 }
        ],
        "Data": {
          "FalsePair": [],
          "TruePair": []
        },
        "EoF": [
          { "State": true, "Time": 568 },
          { "State": false, "Time": 32767 },
          { "State": false, "Time": 32767 },
          { "State": false, "Time": 30543 }
        ]
      }
    ]
  })");

  IrProto proto;
  std::string name = serialiser::fromJson(j, proto);

  EXPECT_EQ(name, "Legacy");
  ASSERT_EQ(proto.getSectionCount(), 2);

  const TimingSection &dataFrame = proto.accessSection(0);
  EXPECT_EQ(dataFrame.getBitCount(), 32);
  EXPECT_EQ(dataFrame.getTiming(), 107870u);
  EXPECT_TRUE(dataFrame.getData().isBoolean());
  EXPECT_EQ(dataFrame.getData().accessQuantum(TimingSectionIrPayload::QUANTUM_TRUE)[1].second, 1662);

  const TimingSection &repeatFrame = proto.accessSection(1);
  EXPECT_EQ(repeatFrame.getBitCount(), 0);
  EXPECT_EQ(repeatFrame.getTiming(), TimingSection::PAUSE_IN_EOF);
  EXPECT_TRUE(repeatFrame.getData().isEmpty());
  EXPECT_EQ(repeatFrame.getEoF().getCount(), 4);
}
