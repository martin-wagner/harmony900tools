// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for irProto::File, IrProto, TimingSection,
 * TimingSectionIrHeader, TimingSectionIrPayload, and Code.
 *
 * Real IrProto.bin structure (verified by hex inspection):
 *   8 protocols, CRC32=0x74693195, data size=455 bytes
 *
 * Round-trip examples below are taken verbatim from UserConfiguration.xml
 * <Code> fields, one per control type present in the file:
 *   FLAT           0x0000640001010000FF00FF010100
 *   SINGLE_SECTION (tx=1) 0x0300F4010100010040040D00000D00
 *   SINGLE_SECTION (tx=3) 0x0100F401030001005F5F07F800
 *   MULTI_SECTION (count=3) 0x0200F4010300030070010002B93600
 *
 * NOTE: no MULTI_SECTION example with a section count of 2 or >3 exists
 * anywhere in UserConfiguration.xml, so those cases are not covered by a
 * round-trip test here. Let us know if you can provide one.
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

using namespace binary;
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
  // LG 82UN85006LA: proto=0, Delay=500, tx=1, repeat=1, ctrl=FLAT
  Code c("0x0000F40101010020DF32CD010100");
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD010100"), Status::OK);

  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getControl(), 0u);  // FLAT
  EXPECT_EQ(c.getDataSectionCount(), 1);
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
  EXPECT_EQ(c.getDataSectionCount(), 1); //repeat is no data section
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
  // Samsung SV-6332X: proto=1, Delay=500, tx=3, repeat=0, ctrl=SINGLE_SECTION
  Code c("0x0100F401030001005F5F778800");
  EXPECT_EQ(c.getIndex(), 1);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getControl(), 1u);  // SINGLE_SECTION
  EXPECT_EQ(c.getDataSectionCount(), 1);
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
  // Philips BDP-2700: proto=2, Delay=500, tx=3, repeat=0, ctrl=MULTI_SECTION(3)
  Code c("0x0200F4010300030070010002B9FF00");
  EXPECT_EQ(c.getIndex(), 2);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getControl(), 3u);  // MULTI_SECTION, dataSectionCount=3
  EXPECT_EQ(c.getDataSectionCount(), 3);
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

TEST(CodeParse, OnkyoFlatDifferentDelay)
{
  // Onkyo TX-NR414: proto=0, Delay=200 (90kHz)
  Code c("0x0000C8000101004B407B84010100");
  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getDelay(), 200);
}

TEST(CodeParse, VuPlusSingleSection)
{
  // Vu+ DUO 2: proto=4, Delay=200, tx=1, no repeat
  Code c("0x0400C80001000100FEB5BFFC00");
  EXPECT_EQ(c.getIndex(), 4);
  EXPECT_EQ(c.getDelay(), 200);
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

// ===========================================================================
// Code – parse: error paths
// ===========================================================================

TEST(CodeParse, FlatPayloadTooSmall)
{
  // header (7 bytes after removing trailing 0) + only 2 payload bytes:
  // parseFlat requires >= 3 (>= 1 data byte + 2 trailer bytes)
  Code c;
  EXPECT_EQ(c.parse("0x0000F401010100010100"), Status::ERROR_SIZE);
}

TEST(CodeParse, FlatTrailerNotOnesRejected)
{
  // last two payload bytes must both be 0x01
  Code c;
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD010200"), Status::ERROR_PAYLOAD_FORMAT);
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD020100"), Status::ERROR_PAYLOAD_FORMAT);
}

TEST(CodeParse, SingleSectionPad7NotZeroRejected)
{
  // code[7] must be 0 for SECTIONS_1
  Code c;
  EXPECT_EQ(c.parse("0x0100F401030001015F5F778800"), Status::ERROR_PAYLOAD_FORMAT);
}

TEST(CodeParse, SingleSectionEmptyPayloadRejected)
{
  // pad byte present but zero payload bytes after it -> ERROR_SIZE
  Code c;
  EXPECT_EQ(c.parse("0x0100F4010300010000"), Status::ERROR_SIZE);
}

TEST(CodeParse, MultiSectionPad7NotZeroRejected)
{
  // code[7] must be 0 for multi-section
  Code c;
  EXPECT_EQ(c.parse("0x0200F401030003017001" "0002" "B9FF00"),
      Status::ERROR_PAYLOAD_FORMAT);
}

TEST(CodeParse, MultiSectionTooFewPayloadBytesRejected)
{
  // ctrl=3 needs 3*2=6 payload bytes, only 4 given
  Code c;
  EXPECT_EQ(c.parse("0x0200F401030003007001000200"), Status::ERROR_SIZE);
}

TEST(CodeParse, RepeatFrameWithMultiTxRejected)
{
  // haveRepeatFrame set AND dataFrameTxCount > 1 is an invalid combination
  Code c;
  EXPECT_EQ(c.parse("0x0000F40103010020DF32CD010100"), Status::ERROR_PAYLOAD_FORMAT);
}

// ===========================================================================
// Code – serialise round-trip
//
// Each case below is a real <Code> value taken from UserConfiguration.xml.
// parse() followed by serialiseStr()/serialiseVec() must reproduce the
// exact original bytes (or the input must be rejected by parse() with an
// error — there is no third outcome).
// ===========================================================================

TEST(CodeRoundTrip, Flat)
{
  const std::string original = "0x0000640001010000FF00FF010100";
  Code c;
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, SingleSection_Tx1)
{
  const std::string original = "0x0300F4010100010040040D00000D00";
  Code c;
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, SingleSection_Tx3)
{
  const std::string original = "0x0100F401030001005F5F07F800";
  Code c;
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, MultiSection_Count3)
{
  const std::string original = "0x0200F4010300030070010002B93600";
  Code c;
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, AllDeviceExamplesFromXml)
{
  // one round-trip per physical-device example listed at the top of this file
  static const char *examples[] = {
    "0x0000F40101010020DF32CD010100", // LG, FLAT
    "0x0100F401030001005F5F778800",   // Samsung, SINGLE_SECTION
    "0x0200F4010300030070010002B9FF00", // Philips, MULTI_SECTION
    "0x0000C8000101004B407B84010100", // Onkyo, FLAT
    "0x0400C80001000100FEB5BFFC00",   // Vu+, SINGLE_SECTION
    "0x0500F40103000100530ACF00",     // XBox, SINGLE_SECTION
    "0x0600F401030001009C0000",       // Led, SINGLE_SECTION
  };

  for (const auto *original : examples) {
    Code c;
    ASSERT_EQ(c.parse(original), Status::OK) << "input: " << original;
    EXPECT_EQ(c.serialiseStr(), original) << "input: " << original;
  }
}

TEST(CodeRoundTrip, VectorFormRoundTrip)
{
  // serialiseVec() must match parse(vector<uint8_t>) the same way
  // serialiseStr() matches parse(string)
  const std::string original = "0x0200F4010300030070010002B93600";
  Code c;
  ASSERT_EQ(c.parse(original), Status::OK);

  auto bytes = c.serialiseVec();
  Code c2;
  ASSERT_EQ(c2.parse(bytes), Status::OK);
  EXPECT_EQ(c2.serialiseStr(), original);
}

// ===========================================================================
// Code – create* / getData()
// ===========================================================================

TEST(CodeCreate, FlatDefaultTxRoundTripsThroughGetData)
{
  // default dataFrameTxCount == 1 after createFlat(): getData() must report
  // exactly the bits that were set, once.
  Code c;
  c.createFlat(0, 36000.0, 32, 0x20DF18E7ULL);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 1u);
  EXPECT_EQ(data[0].second.size(), 32u);
}

TEST(CodeCreate, FlatTxCountMatchesGetDataSize)
{
  // tx=3, no repeat frame, FLAT control, 1 payload byte (0xAA) + 01 01 trailer
  Code c;
  ASSERT_EQ(c.parse("0x0000F401030000AA010100"), Status::OK);

  auto data = c.getData();
  EXPECT_EQ(data.size(), 3u);
}

TEST(CodeCreate, SingleSectionRoundTripsThroughSerialise)
{
  Code c;
  c.createSingleSection(1, 36000.0, 32, 0x5F5F7788ULL);
  auto vec = c.serialiseVec();

  Code c2;
  ASSERT_EQ(c2.parse(vec), Status::OK);
  EXPECT_EQ(c2.getIndex(), 1);
  EXPECT_EQ(c2.getControl(), 1u);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 1u);
  ASSERT_EQ(data[0].second.size(), 32u);
}

TEST(CodeCreate, MultiSectionRoundTripsThroughSerialise)
{
  Code c;
  c.createMultiSection(2, 36000.0, {
    { 16, 0x7001 },
    { 16, 0x0002 },
    { 16, 0xB9FF },
  });
  auto vec = c.serialiseVec();

  Code c2;
  ASSERT_EQ(c2.parse(vec), Status::OK);
  EXPECT_EQ(c2.getIndex(), 2);
  EXPECT_EQ(c2.getControl(), 3u);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 3u);
  EXPECT_EQ(data[0].second.size(), 16u);
}

// ===========================================================================
// Code – bit packing helpers (via createSingleSection round-trip, since
// bytesToBits/bitsToBytes/u64tobits are protected)
// ===========================================================================

TEST(CodeBitPacking, NonByteAlignedBitCountPadsWithZeros)
{
  // 12 bits: 1010 1100 1101 -> 0xAC, 0xD0 (last nibble zero-padded)
  Code c;
  c.createSingleSection(0, 36000.0, 12, 0xACDULL);
  auto vec = c.serialiseVec();

  Code c2;
  ASSERT_EQ(c2.parse(vec), Status::OK);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 1u);
  // bitsToBytes always rounds up to whole bytes -> 2 bytes = 16 bits stored
  EXPECT_EQ(data[0].second.size(), 16u);
}

TEST(CodeBitPacking, SixtyFourBitPayload)
{
  Code c;
  c.createSingleSection(0, 36000.0, 64, 0xFFEEDDCCBBAA9988ULL);
  auto vec = c.serialiseVec();

  Code c2;
  ASSERT_EQ(c2.parse(vec), Status::OK);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 1u);
  ASSERT_EQ(data[0].second.size(), 64u);
  // 0xFF = 11111111
  EXPECT_EQ(data[0].second[0], true);
  EXPECT_EQ(data[0].second[7], true);
  // last byte 0x88 = 10001000
  EXPECT_EQ(data[0].second[56], true);
  EXPECT_EQ(data[0].second[57], false);
}
