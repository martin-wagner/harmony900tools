/*
 * test_irproto.cpp
 *
 * Unit tests for irProto::File, IrProto, TimingSection,
 * TimingSectionIrHeader, TimingSectionIrPayload, and Code.
 *
 * Real IrProto.bin structure (verified by hex inspection):
 *   8 protocols, CRC32=0x74693195, data size=455 bytes
 *
 * Device command examples extracted from UserConfiguration.xml
 * (one per physical device, not per protocol):
 *
 *   LG    82UN85006LA   proto=0  FLAT           0x0000F40101010020DF32CD010100
 *   Samsung SV-6332X   proto=1  SINGLE_SECTION 0x0100F401030001005F5F778800
 *   Philips BDP-2700   proto=2  MULTI_SECTION  0x0200F4010300030070010002B9FF00
 *   Onkyo TX-NR414     proto=0  FLAT           0x0000C8000101004B407B84010100
 *   Vu+ DUO 2          proto=4  SINGLE_SECTION 0x0400C80001000100FEB5BFFC00
 *   Microsoft XBox     proto=5  SINGLE_SECTION 0x0500F40103000100530ACF00
 *   Led RGB-LED-1      proto=6  SINGLE_SECTION 0x0600F401030001009C0000
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "code.h"

using namespace irProto;

// ===========================================================================
// Code – parse
// ===========================================================================

TEST(CodeParse, EmptyStringTooSmall)
{
  Code c;
  EXPECT_EQ(c.parse("0x000000"), Status::ERROR_SIZE);
}

TEST(CodeParse, MissingTrailingZero)
{
  // Valid size but last byte != 0x00
  Code c;
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD0101FF"),
      Status::ERROR_FILE_FORMAT);
}

TEST(CodeParse, FlatHeaderFields_LG)
{
  // LG 82UN85006LA: proto=0, ticks=500, tx=1, repeat=1, ctrl=FLAT
  Code c("0x0000F40101010020DF32CD010100");
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD010100"), Status::OK);

  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getTicks(), 500);
  EXPECT_EQ(c.getControl(), 0u);  // FLAT
  EXPECT_NEAR(c.getClock(), 36000.0, 10.0);
  EXPECT_EQ(c.getRepeatTypeStr(), "Repeat Frame");
}

TEST(CodeParse, FlatSectionCount_LG)
{
  // tx=1 data + 1 repeat section = 2 total sections
  Code c("0x0000F40101010020DF32CD010100");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 2u);
  EXPECT_EQ(data[0].first, 0);   // data section index 0
  EXPECT_EQ(data[1].first, 1);   // repeat section index 1
  EXPECT_TRUE(data[1].second.empty()); // repeat has no bits
}

TEST(CodeParse, FlatBits_LG)
{
  // 0x20DF32CD MSB-first -> 00100000 11011111 00110010 11001101
  Code c("0x0000F40101010020DF32CD010100");
  auto data = c.getData();
  ASSERT_GE(data.size(), 1u);
  const auto &bits = data[0].second;
  ASSERT_EQ(bits.size(), 32u);
  // spot check first byte 0x20 = 00100000
  EXPECT_EQ(bits[0], false);
  EXPECT_EQ(bits[1], false);
  EXPECT_EQ(bits[2], true);
  EXPECT_EQ(bits[7], false);
  // second byte 0xDF = 11011111
  EXPECT_EQ(bits[8], true);
  EXPECT_EQ(bits[9], true);
  EXPECT_EQ(bits[10], false);
}

TEST(CodeParse, SingleSectionHeaderFields_Samsung)
{
  // Samsung SV-6332X: proto=1, ticks=500, tx=3, repeat=0, ctrl=SINGLE_SECTION
  Code c("0x0100F401030001005F5F778800");
  EXPECT_EQ(c.getIndex(), 1);
  EXPECT_EQ(c.getTicks(), 500);
  EXPECT_EQ(c.getControl(), 1u);  // SINGLE_SECTION
  EXPECT_EQ(c.getRepeatTypeStr(), "Repeats: 3");
}

TEST(CodeParse, SingleSectionCount_Samsung)
{
  // tx=3, no repeat -> 3x section[0]
  Code c("0x0100F401030001005F5F778800");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 3u);
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 0);
  EXPECT_EQ(data[2].first, 0);
}

TEST(CodeParse, SingleSectionBits_Samsung)
{
  // 0x5F5F7788 -> 01011111 01011111 01110111 10001000 = 32 bits
  Code c("0x0100F401030001005F5F778800");
  auto data = c.getData();
  ASSERT_GE(data.size(), 1u);
  const auto &bits = data[0].second;
  ASSERT_EQ(bits.size(), 32u);
  // 0x5F = 01011111
  EXPECT_EQ(bits[0], false);
  EXPECT_EQ(bits[1], true);
  EXPECT_EQ(bits[2], false);
  EXPECT_EQ(bits[3], true);
}

TEST(CodeParse, MultiSectionHeaderFields_Philips)
{
  // Philips BDP-2700: proto=2, ticks=500, tx=3, repeat=0, ctrl=MULTI_SECTION(3)
  Code c("0x0200F4010300030070010002B9FF00");
  EXPECT_EQ(c.getIndex(), 2);
  EXPECT_EQ(c.getTicks(), 500);
  EXPECT_EQ(c.getControl(), 3u);  // MULTI_SECTION, dataSectionCount=3
}

TEST(CodeParse, MultiSectionCount_Philips)
{
  // tx=3, 3 sections each -> 9 total sections
  Code c("0x0200F4010300030070010002B9FF00");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 9u);
  // first tx: sections 0,1,2
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 1);
  EXPECT_EQ(data[2].first, 2);
}

TEST(CodeParse, MultiSectionBits_Philips)
{
  // sec0=0x7001 -> 0111000000000001
  Code c("0x0200F4010300030070010002B9FF00");
  auto data = c.getData();
  ASSERT_GE(data.size(), 3u);
  const auto &sec0 = data[0].second;
  ASSERT_EQ(sec0.size(), 16u);
  // 0x70 = 01110000
  EXPECT_EQ(sec0[0], false);
  EXPECT_EQ(sec0[1], true);
  EXPECT_EQ(sec0[2], true);
  EXPECT_EQ(sec0[3], true);
  // sec1=0x0002 -> 0000000000000010
  const auto &sec1 = data[1].second;
  ASSERT_EQ(sec1.size(), 16u);
  EXPECT_EQ(sec1[14], true);
  EXPECT_EQ(sec1[15], false);
}

TEST(CodeParse, OnkyoFlatDifferentTicks)
{
  // Onkyo TX-NR414: proto=0, ticks=200 (90kHz)
  Code c("0x0000C8000101004B407B84010100");
  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getTicks(), 200);
  EXPECT_NEAR(c.getClock(), 90000.0, 10.0);
}

TEST(CodeParse, VuPlusSingleSection)
{
  // Vu+ DUO 2: proto=4, ticks=200, tx=1, no repeat
  Code c("0x0400C80001000100FEB5BFFC00");
  EXPECT_EQ(c.getIndex(), 4);
  EXPECT_EQ(c.getTicks(), 200);
  EXPECT_EQ(c.getControl(), 1u);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 1u);  // tx=1, no repeat
  ASSERT_EQ(data[0].second.size(), 32u);
  // 0xFE = 11111110
  EXPECT_EQ(data[0].second[0], true);
  EXPECT_EQ(data[0].second[7], false);
}

TEST(CodeParse, XBoxSingleSection24Bits)
{
  // XBox: proto=5, 24 bits (3 payload bytes)
  Code c("0x0500F40103000100530ACF00");
  EXPECT_EQ(c.getIndex(), 5);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 3u); // tx=3
  ASSERT_EQ(data[0].second.size(), 24u);
}

TEST(CodeParse, LedSingleSection16Bits)
{
  // Led RGB-LED-1: proto=6, 16 bits (2 payload bytes)
  Code c("0x0600F401030001009C0000");
  EXPECT_EQ(c.getIndex(), 6);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 3u); // tx=3
  ASSERT_EQ(data[0].second.size(), 16u);
  // 0x9C = 10011100
  EXPECT_EQ(data[0].second[0], true);
  EXPECT_EQ(data[0].second[1], false);
  EXPECT_EQ(data[0].second[2], false);
  EXPECT_EQ(data[0].second[3], true);
}

TEST(CodeParse, GetDataForIrProto)
{
  Code c("0x0000F40101010020DF32CD010100");
  auto data = c.getData();
  // getData returns IrProto::Data, which is vector<pair<int, vector<bool>>>
  EXPECT_FALSE(data.empty());
}
