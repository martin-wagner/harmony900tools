// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for irProto::Code.
 *
 * A code is: index (2) | delay (2) | repeats (1) | start group | repeat group | finish group
 * Each group is: record count (1) | { section index (1) | section data } per record.
 * The data length of a section comes from the protocol (symbols * bits per symbol, rounded up to bytes).
 *
 * The protocols are built in the test fixtures below, they only describe the section
 * layout (symbol count, states per symbol) of the protocols used by the codes.
 *
 * Code examples are taken from UserConfiguration.xml <Code> fields:
 *
 *   LG    82UN85006LA   proto=0  NEC style   0x0000F40101010020DF32CD010100
 *   Samsung SV-6332X    proto=1  repeat only  0x0100F401030001005F5F778800
 *   Philips BDP-2700    proto=2  RC-6 style   0x0200F4010300030070010002B9FF00
 *   Onkyo TX-NR414      proto=0  NEC style   0x0000C8000101004B407B84010100
 *   Vu+ DUO 2           proto=4  repeat only  0x0400C80001000100FEB5BFFC00
 *   Microsoft XBox      proto=5  repeat only  0x0500F40103000100530ACF00
 *   Led RGB-LED-1       proto=6  repeat only  0x0600F401030001009C0000
 *   Belgacom ISB6030    proto=3  quad, repeat + finish
 *   B&O Beocenter 9500  proto=5/6 quad, start / repeat
 *   Artemide Metamorfosi proto=7 quad
 */

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "code.h"

using namespace binary;
using namespace irProto;

namespace {

using Symbols = std::vector<uint8_t>;

/** section with <symbols> symbols and <quanta> states per symbol (0 = no payload) */
TimingSection makeSection(uint16_t symbols, uint8_t quanta)
{
  TimingSectionIrPayload payload;

  for (uint8_t q = 0; q < quanta; q++) {
    std::vector<Item> timings = { { true, 560 }, { false, static_cast<uint16_t>(560 * (q + 1)) } };
    payload.setQuantum(q, timings);
  }

  TimingSection section;
  section.setBitCount(symbols);
  section.setData(payload);
  return section;
}

IrProto makeProto(const std::vector<TimingSection> &sections)
{
  return IrProto(27778, sections);
}

// data section + repeat frame without data (NEC style)
IrProto protoNec()
{
  return makeProto( { makeSection(32, 2), makeSection(0, 0) });
}

// one section, 2 states per symbol
IrProto protoSingle(uint16_t symbols)
{
  return makeProto( { makeSection(symbols, 2) });
}

// 4 / 1 / 16 symbols
IrProto protoRc6()
{
  return makeProto( { makeSection(4, 2), makeSection(1, 2), makeSection(16, 2) });
}

IrProto protoMulti16()
{
  return makeProto( { makeSection(16, 2), makeSection(16, 2), makeSection(16, 2) });
}

IrProto protoBelgacom()
{
  return makeProto( { makeSection(16, 4), makeSection(16, 4), makeSection(16, 4) });
}

IrProto protoBeoDual()
{
  return makeProto( { makeSection(17, 4), makeSection(1, 2), makeSection(17, 4), makeSection(20, 4) });
}

IrProto protoGalaxis()
{
  return makeProto( { makeSection(8, 4), makeSection(1, 2), makeSection(7, 4), makeSection(1, 2) });
}

}

// ===========================================================================
// Code - symbol / byte helpers
// ===========================================================================

TEST(CodeHelpers, BitsPerSymbol)
{
  Code c(0, protoSingle(8));
  EXPECT_EQ(c.getBitsPerSymbol(0), 0);
  EXPECT_EQ(c.getBitsPerSymbol(1), 0);
  EXPECT_EQ(c.getBitsPerSymbol(2), 1);
  EXPECT_EQ(c.getBitsPerSymbol(3), 2);
  EXPECT_EQ(c.getBitsPerSymbol(4), 2);
  EXPECT_EQ(c.getBitsPerSymbol(5), 3);
  EXPECT_EQ(c.getBitsPerSymbol(16), 4);
}

TEST(CodeHelpers, DataBytes)
{
  Code c(0, protoSingle(8));
  EXPECT_EQ(c.getDataBytes(0, 2), 0);
  EXPECT_EQ(c.getDataBytes(8, 0), 0);
  EXPECT_EQ(c.getDataBytes(32, 1), 4);
  EXPECT_EQ(c.getDataBytes(48, 1), 6);
  EXPECT_EQ(c.getDataBytes(20, 1), 3);
  EXPECT_EQ(c.getDataBytes(17, 2), 5);
  EXPECT_EQ(c.getDataBytes(7, 2), 2);
  EXPECT_EQ(c.getDataBytes(16, 4), 8);
}

TEST(CodeHelpers, BytesToListBoolean)
{
  const Symbols expected = { 1, 0, 1, 0, 0, 1, 0, 1 };
  EXPECT_EQ(Code::bytesToList(8, 1, { 0xA5 }), expected);
}

TEST(CodeHelpers, BytesToListQuad)
{
  // Beocenter 9500 key "0"
  const Symbols expected = { 0, 1, 1, 1, 1, 1, 1, 1, 2, 0, 1, 1, 1, 1, 1, 1, 1 };
  EXPECT_EQ(Code::bytesToList(17, 2, { 0x15, 0x55, 0x85, 0x55, 0x40 }), expected);
}

TEST(CodeHelpers, BytesToListQuadBelgacom)
{
  const Symbols expected = { 0, 2, 0, 2, 3, 0, 0, 0, 0, 2, 1, 2, 1, 0, 3, 1 };
  EXPECT_EQ(Code::bytesToList(16, 2, { 0x22, 0xC0, 0x26, 0x4D }), expected);
}

TEST(CodeHelpers, BytesToListPaddingIgnored)
{
  // 14 bits of 0x9A0C, the 2 padding bits are not a symbol
  const Symbols expected = { 2, 1, 2, 2, 0, 0, 3 };
  EXPECT_EQ(Code::bytesToList(7, 2, { 0x9A, 0x0C }), expected);
}

TEST(CodeHelpers, BytesToListMissingDataIsZero)
{
  const Symbols expected = { 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
  EXPECT_EQ(Code::bytesToList(16, 1, { 0xA5 }), expected);
}

TEST(CodeHelpers, ListToBytesBoolean)
{
  // 12 symbols: 1010 1100 1101 -> AC D0 (last nibble zero padded)
  const Symbols values = { 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1 };
  const std::vector<uint8_t> expected = { 0xAC, 0xD0 };
  EXPECT_EQ(Code::listToBytes(1, values), expected);
}

TEST(CodeHelpers, ListToBytesQuad)
{
  const Symbols values = { 0, 1, 1, 1, 1, 1, 1, 1, 2, 0, 1, 1, 1, 1, 1, 1, 1 };
  const std::vector<uint8_t> expected = { 0x15, 0x55, 0x85, 0x55, 0x40 };
  EXPECT_EQ(Code::listToBytes(2, values), expected);
}

TEST(CodeHelpers, ListToBytesQuadPadding)
{
  const Symbols values = { 2, 1, 2, 2, 0, 0, 3 };
  const std::vector<uint8_t> expected = { 0x9A, 0x0C };
  EXPECT_EQ(Code::listToBytes(2, values), expected);
}

TEST(CodeHelpers, ListToBytesEmpty)
{
  EXPECT_TRUE(Code::listToBytes(2, { }).empty());
}

TEST(CodeHelpers, BytesListRoundTrip)
{
  const std::vector<uint8_t> bytes = { 0x22, 0xC0, 0xA6, 0xF7 };
  auto list = Code::bytesToList(16, 2, bytes);
  EXPECT_EQ(Code::listToBytes(2, list), bytes);
}

TEST(CodeHelpers, U64BitsToList)
{
  const Symbols expected = { 1, 0, 1, 0, 0, 1, 0, 1 };
  EXPECT_EQ(Code::u64bitsToList(8, 0xA5), expected);
}

TEST(CodeHelpers, U64BitsToListLeadingZeros)
{
  const Symbols expected = { 0, 0, 0, 0, 1, 0, 1, 1 };
  EXPECT_EQ(Code::u64bitsToList(8, 0x0B), expected);
}

TEST(CodeHelpers, U64BitsToList64)
{
  auto list = Code::u64bitsToList(64, 0x8000000000000001ULL);
  ASSERT_EQ(list.size(), 64u);
  EXPECT_EQ(list.front(), 1);
  EXPECT_EQ(list.back(), 1);
  EXPECT_EQ(list[1], 0);
  EXPECT_EQ(list[62], 0);
}

TEST(CodeHelpers, ListToU64Bits)
{
  const Symbols values = { 1, 0, 1, 0, 0, 1, 0, 1 };
  EXPECT_EQ(Code::listToU64bits(values), 0xA5ULL);
}

TEST(CodeHelpers, U64BitsRoundTrip)
{
  EXPECT_EQ(Code::listToU64bits(Code::u64bitsToList(48, 0x40040D00000DULL)), 0x40040D00000DULL);
  EXPECT_EQ(Code::listToU64bits(Code::u64bitsToList(64, 0xFFEEDDCCBBAA9988ULL)), 0xFFEEDDCCBBAA9988ULL);
}

// ===========================================================================
// Code - parse
// ===========================================================================

TEST(CodeParse, EmptyStringTooSmall)
{
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x000000"), Status::ERROR_SIZE);
}

TEST(CodeParse, IndexMismatch)
{
  Code c(1, protoNec());
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD010100"), Status::ERROR_INDEX);
}

TEST(CodeParse, InvalidFinishGroup)
{
  // finish group announces 255 records, but nothing follows
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD0101FF"), Status::ERROR_SIZE);
}

TEST(CodeParse, FlatHeaderFields_LG)
{
  // LG 82UN85006LA: proto=0, Delay=500, repeats=1, data in start group, empty repeat frame in repeat group
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD010100"), Status::OK);

  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 1);
  EXPECT_EQ(c.accessStartGroups().size(), 1u);
  EXPECT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);
}

TEST(CodeParse, FlatSectionCount_LG)
{
  // 1 data frame + 1 repeat frame = 2 sections
  Code c(0, protoNec(), "0x0000F40101010020DF32CD010100");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 2u);
  EXPECT_EQ(data[0].first, 0);   // data section index 0
  EXPECT_EQ(data[1].first, 1);   // repeat section index 1
  EXPECT_TRUE(data[1].second.empty()); // repeat has no symbols
}

TEST(CodeParse, FlatGroupLayout_LG)
{
  Code c(0, protoNec(), "0x0000F40101010020DF32CD010100");
  ASSERT_EQ(c.accessStartGroups().size(), 1u);
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessStartGroups()[0].getSection(), 0);
  EXPECT_EQ(c.accessStartGroups()[0].getBits(), 1);
  EXPECT_EQ(c.accessStartGroups()[0].getData().size(), 32u);
  EXPECT_EQ(c.accessRepeatGroups()[0].getSection(), 1);
  EXPECT_TRUE(c.accessRepeatGroups()[0].getData().empty());
}

TEST(CodeParse, FlatBits_LG)
{
  // 0x20DF32CD MSB-first -> 00100000 11011111 00110010 11001101
  Code c(0, protoNec(), "0x0000F40101010020DF32CD010100");
  auto data = c.getData();
  ASSERT_GE(data.size(), 1u);
  const auto &bits = data[0].second;
  ASSERT_EQ(bits.size(), 32u);
  // first byte 0x20 = 00100000
  EXPECT_EQ(bits[0], 0);
  EXPECT_EQ(bits[1], 0);
  EXPECT_EQ(bits[2], 1);
  EXPECT_EQ(bits[7], 0);
  // second byte 0xDF = 11011111
  EXPECT_EQ(bits[8], 1);
  EXPECT_EQ(bits[9], 1);
  EXPECT_EQ(bits[10], 0);
}

TEST(CodeParse, SingleSectionHeaderFields_Samsung)
{
  // Samsung SV-6332X: proto=1, Delay=500, repeats=3, data in repeat group
  Code c(1, protoSingle(32), "0x0100F401030001005F5F778800");
  EXPECT_EQ(c.getIndex(), 1);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 3);
  EXPECT_EQ(c.accessStartGroups().size(), 0u);
  EXPECT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);
}

TEST(CodeParse, SingleSectionCount_Samsung)
{
  // repeats=3 -> 3x section[0]
  Code c(1, protoSingle(32), "0x0100F401030001005F5F778800");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 3u);
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 0);
  EXPECT_EQ(data[2].first, 0);
}

TEST(CodeParse, SingleSectionBits_Samsung)
{
  // 0x5F5F7788 -> 01011111 01011111 01110111 10001000 = 32 bits
  Code c(1, protoSingle(32), "0x0100F401030001005F5F778800");
  auto data = c.getData();
  ASSERT_GE(data.size(), 1u);
  const auto &bits = data[0].second;
  ASSERT_EQ(bits.size(), 32u);
  // 0x5F = 01011111
  EXPECT_EQ(bits[0], 0);
  EXPECT_EQ(bits[1], 1);
  EXPECT_EQ(bits[2], 0);
  EXPECT_EQ(bits[3], 1);
}

TEST(CodeParse, MultiSectionHeaderFields_Philips)
{
  // Philips BDP-2700: proto=2, Delay=500, repeats=3, repeat group with 3 sections
  Code c(2, protoRc6(), "0x0200F4010300030070010002B9FF00");
  EXPECT_EQ(c.getIndex(), 2);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 3);
  EXPECT_EQ(c.accessStartGroups().size(), 0u);
  EXPECT_EQ(c.accessRepeatGroups().size(), 3u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);
}

TEST(CodeParse, MultiSectionCount_Philips)
{
  // repeats=3, 3 sections each -> 9 total sections
  Code c(2, protoRc6(), "0x0200F4010300030070010002B9FF00");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 9u);
  // first repetition: sections 0,1,2
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 1);
  EXPECT_EQ(data[2].first, 2);
  // last repetition: sections 0,1,2
  EXPECT_EQ(data[6].first, 0);
  EXPECT_EQ(data[7].first, 1);
  EXPECT_EQ(data[8].first, 2);
}

TEST(CodeParse, MultiSectionBits_Philips)
{
  // sec0 = 0x70 -> 4 symbols 0111, sec1 = 0x00 -> 1 symbol 0, sec2 = 0xB9FF -> 16 symbols
  Code c(2, protoRc6(), "0x0200F4010300030070010002B9FF00");
  auto data = c.getData();
  ASSERT_GE(data.size(), 3u);

  const Symbols sec0 = { 0, 1, 1, 1 };
  EXPECT_EQ(data[0].second, sec0);

  const Symbols sec1 = { 0 };
  EXPECT_EQ(data[1].second, sec1);

  const Symbols sec2 = { 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  EXPECT_EQ(data[2].second, sec2);
}

TEST(CodeParse, OnkyoFlatDifferentDelay)
{
  // Onkyo TX-NR414: proto=0, Delay=200
  Code c(0, protoNec(), "0x0000C8000101004B407B84010100");
  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getDelay(), 200);
  EXPECT_EQ(c.getRepeats(), 1);
}

TEST(CodeParse, VuPlusSingleSection)
{
  // Vu+ DUO 2: proto=4, Delay=200, repeats=1
  Code c(4, protoSingle(32), "0x0400C80001000100FEB5BFFC00");
  EXPECT_EQ(c.getIndex(), 4);
  EXPECT_EQ(c.getDelay(), 200);
  EXPECT_EQ(c.getRepeats(), 1);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 1u);
  ASSERT_EQ(data[0].second.size(), 32u);
  // 0xFE = 11111110
  EXPECT_EQ(data[0].second[0], 1);
  EXPECT_EQ(data[0].second[7], 0);
}

TEST(CodeParse, XBoxSingleSection24Bits)
{
  // XBox: proto=5, 24 symbols (3 payload bytes)
  Code c(5, protoSingle(24), "0x0500F40103000100530ACF00");
  EXPECT_EQ(c.getIndex(), 5);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 3u); // repeats=3
  ASSERT_EQ(data[0].second.size(), 24u);
}

TEST(CodeParse, LedSingleSection16Bits)
{
  // Led RGB-LED-1: proto=6, 16 symbols (2 payload bytes)
  Code c(6, protoSingle(16), "0x0600F401030001009C0000");
  EXPECT_EQ(c.getIndex(), 6);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 3u); // repeats=3
  ASSERT_EQ(data[0].second.size(), 16u);
  // 0x9C = 10011100
  EXPECT_EQ(data[0].second[0], 1);
  EXPECT_EQ(data[0].second[1], 0);
  EXPECT_EQ(data[0].second[2], 0);
  EXPECT_EQ(data[0].second[3], 1);
}

TEST(CodeParse, Panasonic48BitContent)
{
  Code c(3, protoSingle(48), "0x0300F4010100010040040D00000D00");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 1u);
  const std::vector<uint8_t> expected = { 0x40, 0x04, 0x0D, 0x00, 0x00, 0x0D };
  EXPECT_EQ(Code::listToBytes(1, data[0].second), expected);
}

TEST(CodeParse, GetDataForIrProto)
{
  Code c(0, protoNec(), "0x0000F40101010020DF32CD010100");
  auto data = c.getData();
  EXPECT_FALSE(data.empty());
}

TEST(CodeParse, ConstructFromVector)
{
  const std::vector<uint8_t> bytes = { 0x00, 0x00, 0xF4, 0x01, 0x01, 0x01, 0x00, 0x20, 0xDF, 0x32, 0xCD, 0x01, 0x01, 0x00 };
  Code c(0, protoNec(), bytes);
  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 1);
  EXPECT_EQ(c.accessStartGroups().size(), 1u);
  EXPECT_EQ(c.accessRepeatGroups().size(), 1u);
}

TEST(CodeParse, ParseOverwritesExisting)
{
  Code c(0, protoNec(), "0x0000F40101010020DF32CD010100");
  ASSERT_EQ(c.parse("0x0000C8000101004B407B84010100"), Status::OK);
  EXPECT_EQ(c.getDelay(), 200);
  EXPECT_EQ(c.accessStartGroups().size(), 1u);
  EXPECT_EQ(c.accessRepeatGroups().size(), 1u);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 2u);
  const std::vector<uint8_t> expected = { 0x4B, 0x40, 0x7B, 0x84 };
  EXPECT_EQ(Code::listToBytes(1, data[0].second), expected);
}

TEST(CodeParse, AllGroupsEmpty)
{
  Code c(0, protoNec());
  ASSERT_EQ(c.parse("0x0000F40101000000"), Status::OK);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 1);
  EXPECT_TRUE(c.accessStartGroups().empty());
  EXPECT_TRUE(c.accessRepeatGroups().empty());
  EXPECT_TRUE(c.accessFinishGroups().empty());
  EXPECT_TRUE(c.getData().empty());
}

TEST(CodeParse, FinishGroupParsedAsFinish)
{
  // Belgacom ISB6030: repeat group with 1 frame, finish group with 2 frames
  Code c(3, protoBelgacom());
  ASSERT_EQ(c.parse("0x0300F4010100010022C026F7020122C026F70222C0A6F7"), Status::OK);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 1);
  EXPECT_EQ(c.accessStartGroups().size(), 0u);
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  ASSERT_EQ(c.accessFinishGroups().size(), 2u);
  EXPECT_EQ(c.accessRepeatGroups()[0].getSection(), 0);
  EXPECT_EQ(c.accessFinishGroups()[0].getSection(), 1);
  EXPECT_EQ(c.accessFinishGroups()[1].getSection(), 2);
}

TEST(CodeParse, QuadSymbols_Belgacom)
{
  Code c(3, protoBelgacom(), "0x0300F4010100010022C026F7020122C026F70222C0A6F7");
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  ASSERT_EQ(c.accessFinishGroups().size(), 2u);
  EXPECT_EQ(c.accessRepeatGroups()[0].getBits(), 2);

  const Symbols frame = { 0, 2, 0, 2, 3, 0, 0, 0, 0, 2, 1, 2, 3, 3, 1, 3 };
  EXPECT_EQ(c.accessRepeatGroups()[0].getData(), frame);
  EXPECT_EQ(c.accessFinishGroups()[0].getData(), frame);

  // last frame differs in the 9th symbol
  const Symbols lastFrame = { 0, 2, 0, 2, 3, 0, 0, 0, 2, 2, 1, 2, 3, 3, 1, 3 };
  EXPECT_EQ(c.accessFinishGroups()[1].getData(), lastFrame);
}

TEST(CodeParse, GetDataOrder_Belgacom)
{
  // start, repeat * repeats, finish
  Code c(3, protoBelgacom(), "0x0300F4010100010022C026F7020122C026F70222C0A6F7");
  auto data = c.getData();
  ASSERT_EQ(data.size(), 3u);
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 1);
  EXPECT_EQ(data[2].first, 2);

  c.setRepeats(2);
  data = c.getData();
  ASSERT_EQ(data.size(), 4u);
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 0);
  EXPECT_EQ(data[2].first, 1);
  EXPECT_EQ(data[3].first, 2);
}

TEST(CodeParse, StartAndRepeatGroups_BeoDual)
{
  // B&O Beocenter 9500, 17 symbol quad frames, start = frame + gap symbol, repeat = second frame
  Code c(6, protoBeoDual(), "0x0600F401010200155594914001000102155589514000");
  EXPECT_EQ(c.getIndex(), 6);
  ASSERT_EQ(c.accessStartGroups().size(), 2u);
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);

  EXPECT_EQ(c.accessStartGroups()[0].getSection(), 0);
  EXPECT_EQ(c.accessStartGroups()[1].getSection(), 1);
  EXPECT_EQ(c.accessRepeatGroups()[0].getSection(), 2);

  const Symbols startFrame = { 0, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 2, 1, 0, 1, 1 };
  const Symbols gap = { 0 };
  const Symbols repeatFrame = { 0, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 1, 1, 1, 0, 1, 1 };
  EXPECT_EQ(c.accessStartGroups()[0].getData(), startFrame);
  EXPECT_EQ(c.accessStartGroups()[1].getData(), gap);
  EXPECT_EQ(c.accessRepeatGroups()[0].getData(), repeatFrame);
}

TEST(CodeParse, FrameInStartGroup_Beocenter)
{
  Code c(5, makeProto( { makeSection(17, 4) }), "0x0500F40101010015558555400000");
  EXPECT_EQ(c.accessStartGroups().size(), 1u);
  EXPECT_EQ(c.accessRepeatGroups().size(), 0u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);
}

TEST(CodeParse, QuadSymbols_Beocenter)
{
  Code c(5, makeProto( { makeSection(17, 4) }), "0x0500F40101010015558555400000");
  ASSERT_EQ(c.accessStartGroups().size(), 1u);
  EXPECT_EQ(c.accessStartGroups()[0].getBits(), 2);
  const Symbols expected = { 0, 1, 1, 1, 1, 1, 1, 1, 2, 0, 1, 1, 1, 1, 1, 1, 1 };
  EXPECT_EQ(c.accessStartGroups()[0].getData(), expected);
}

TEST(CodeParse, FrameInRepeatGroup_Beocenter)
{
  Code c(5, makeProto( { makeSection(17, 4) }), "0x0500F40101000100155591864000");
  EXPECT_EQ(c.accessStartGroups().size(), 0u);
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);
  const Symbols expected = { 0, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0, 1, 2, 0, 1, 2, 1 };
  EXPECT_EQ(c.accessRepeatGroups()[0].getData(), expected);
}

TEST(CodeParse, FrameInStartOrRepeatGroupSendsOnce_Beocenter)
{
  Code flat(5, makeProto( { makeSection(17, 4) }), "0x0500F40101010015558555400000");
  EXPECT_EQ(flat.getData().size(), 1u);

  Code single(5, makeProto( { makeSection(17, 4) }), "0x0500F40101000100155591864000");
  EXPECT_EQ(single.getData().size(), 1u);
}

TEST(CodeParse, ToggleSections_Galaxis)
{
  // 8 symbols quad, 1 bit toggle, 7 symbols quad, repeat group with 3 sections
  Code c(4, protoGalaxis(), "0x0400F4010100030022C00100029A0C00");
  ASSERT_EQ(c.accessRepeatGroups().size(), 3u);
  EXPECT_EQ(c.accessStartGroups().size(), 0u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);

  EXPECT_EQ(c.accessRepeatGroups()[0].getSection(), 0);
  EXPECT_EQ(c.accessRepeatGroups()[1].getSection(), 1);
  EXPECT_EQ(c.accessRepeatGroups()[2].getSection(), 2);

  EXPECT_EQ(c.accessRepeatGroups()[0].getBits(), 2);
  EXPECT_EQ(c.accessRepeatGroups()[1].getBits(), 1);
  EXPECT_EQ(c.accessRepeatGroups()[2].getBits(), 2);

  const Symbols sec0 = { 0, 2, 0, 2, 3, 0, 0, 0 };
  const Symbols sec1 = { 0 };
  const Symbols sec2 = { 2, 1, 2, 2, 0, 0, 3 };
  EXPECT_EQ(c.accessRepeatGroups()[0].getData(), sec0);
  EXPECT_EQ(c.accessRepeatGroups()[1].getData(), sec1);
  EXPECT_EQ(c.accessRepeatGroups()[2].getData(), sec2);
}

TEST(CodeParse, Quad14Symbols_Artemide)
{
  // Artemide Metamorfosi: delay 100, repeats 3, 14 quad symbols = 28 bits + 4 padding bits
  Code c(7, makeProto( { makeSection(14, 4) }), "0x07006400030001008899999000");
  EXPECT_EQ(c.getIndex(), 7);
  EXPECT_EQ(c.getDelay(), 100);
  EXPECT_EQ(c.getRepeats(), 3);
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessRepeatGroups()[0].getData().size(), 14u);
}

TEST(CodeParse, Quad14SymbolsContent_Artemide)
{
  Code c(7, makeProto( { makeSection(14, 4) }), "0x07006400030001008899999000");
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessRepeatGroups()[0].getBits(), 2);
  const Symbols expected = { 2, 0, 2, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 };
  EXPECT_EQ(c.accessRepeatGroups()[0].getData(), expected);
}

TEST(CodeParse, Sony20BitPadding)
{
  // 20 symbols = 2.5 bytes, last nibble is padding
  Code c(8, protoSingle(20), "0x0800F4010100010090B47000");
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  const Symbols expected = { 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1 };
  EXPECT_EQ(c.accessRepeatGroups()[0].getData(), expected);
}

// ===========================================================================
// Code - parse: static index
// ===========================================================================

TEST(CodeParseIndex, FromString)
{
  int index = -2;
  EXPECT_EQ(Code::parse(std::string("0x07006400030001008899999000"), index), Status::OK);
  EXPECT_EQ(index, 7);
}

TEST(CodeParseIndex, FromVector)
{
  int index = -2;
  const std::vector<uint8_t> bytes = { 0x03, 0x00, 0xF4, 0x01, 0x01, 0x00, 0x00, 0x00 };
  EXPECT_EQ(Code::parse(bytes, index), Status::OK);
  EXPECT_EQ(index, 3);
}

TEST(CodeParseIndex, TooSmall)
{
  int index = 5;
  EXPECT_EQ(Code::parse(std::string("0x0700"), index), Status::ERROR_SIZE);
  EXPECT_EQ(index, -1);
}

// ===========================================================================
// Code - parse: error paths
// ===========================================================================

TEST(CodeParse, FrameDataTruncated)
{
  // start group record needs 4 data bytes, only 3 given
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x0000F401010100010100"), Status::ERROR_SIZE);
}

TEST(CodeParse, RepeatGroupMissing)
{
  // start group complete, repeat and finish group counts missing
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD00"), Status::ERROR_SIZE);
}

TEST(CodeParse, RepeatSectionIndexOutOfRange)
{
  // protocol has sections 0 and 1 only
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD010200"), Status::ERROR_PAYLOAD_FORMAT);
}

TEST(CodeParse, SecondRecordWithoutData)
{
  // repeat group announces 2 records, the second needs 4 data bytes
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x0000F40101010020DF32CD020100"), Status::ERROR_SIZE);
}

TEST(CodeParse, SingleSectionIndexOutOfRange)
{
  // protocol has section 0 only
  Code c(1, protoSingle(32));
  EXPECT_EQ(c.parse("0x0100F401030001015F5F778800"), Status::ERROR_PAYLOAD_FORMAT);
}

TEST(CodeParse, SingleSectionEmptyPayloadRejected)
{
  // section index present but no payload bytes after it
  Code c(1, protoSingle(32));
  EXPECT_EQ(c.parse("0x0100F4010300010000"), Status::ERROR_SIZE);
}

TEST(CodeParse, MultiSectionIndexOutOfRange)
{
  Code c(2, protoRc6());
  EXPECT_EQ(c.parse("0x0200F40103000307700100" "02" "B9FF00"), Status::ERROR_PAYLOAD_FORMAT);
}

TEST(CodeParse, MultiSectionTooFewPayloadBytesRejected)
{
  // 3 records announced, the last one needs 2 data bytes and has 1
  Code c(2, protoRc6());
  EXPECT_EQ(c.parse("0x0200F401030003007001000200"), Status::ERROR_SIZE);
}

TEST(CodeParse, TrailingBytesRejected)
{
  Code c(0, protoNec());
  EXPECT_EQ(c.parse("0x0000F4010100000000"), Status::ERROR_SIZE);
}

TEST(CodeParse, RepeatFrameWithMultipleRepeats)
{
  // repeat frame (no data) is sent 3 times after the data frame
  Code c(0, protoNec());
  ASSERT_EQ(c.parse("0x0000F40103010020DF32CD010100"), Status::OK);
  EXPECT_EQ(c.getRepeats(), 3);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 4u);
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 1);
  EXPECT_EQ(data[2].first, 1);
  EXPECT_EQ(data[3].first, 1);
}

TEST(CodeParse, ResetAfterFailedParse)
{
  Code c(0, protoNec(), "0x0000F40101010020DF32CD010100");
  ASSERT_NE(c.parse("0x0000F4010101"), Status::OK);
  EXPECT_TRUE(c.accessStartGroups().empty());
  EXPECT_TRUE(c.accessRepeatGroups().empty());
  EXPECT_TRUE(c.accessFinishGroups().empty());
}

// ===========================================================================
// Code - serialise round-trip
//
// Each case below is a real <Code> value taken from UserConfiguration.xml.
// parse() followed by serialiseStr()/serialiseVec() must reproduce the
// exact original bytes.
// ===========================================================================

TEST(CodeRoundTrip, Flat)
{
  const std::string original = "0x0000640001010000FF00FF010100";
  Code c(0, protoNec());
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, SingleSection_Tx1)
{
  const std::string original = "0x0300F4010100010040040D00000D00";
  Code c(3, protoSingle(48));
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, SingleSection_Tx3)
{
  const std::string original = "0x0100F401030001005F5F07F800";
  Code c(1, protoSingle(32));
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, MultiSection_Count3)
{
  const std::string original = "0x0200F4010300030070010002B93600";
  Code c(2, protoRc6());
  ASSERT_EQ(c.parse(original), Status::OK);
  EXPECT_EQ(c.serialiseStr(), original);
}

TEST(CodeRoundTrip, BinaryDeviceExamples)
{
  // one round-trip per physical-device example listed at the top of this file
  struct Example {
      uint16_t index;
      IrProto protocol;
      std::string code;
  };

  const std::vector<Example> examples = {
    { 0, protoNec(), "0x0000F40101010020DF32CD010100" },           // LG
    { 1, protoSingle(32), "0x0100F401030001005F5F778800" },        // Samsung
    { 2, protoRc6(), "0x0200F4010300030070010002B9FF00" },         // Philips
    { 0, protoNec(), "0x0000C8000101004B407B84010100" },           // Onkyo
    { 4, protoSingle(32), "0x0400C80001000100FEB5BFFC00" },        // Vu+
    { 5, protoSingle(24), "0x0500F40103000100530ACF00" },          // XBox
    { 6, protoSingle(16), "0x0600F401030001009C0000" },            // Led
  };

  for (const auto &example : examples) {
    Code c(example.index, example.protocol);
    ASSERT_EQ(c.parse(example.code), Status::OK) << "input: " << example.code;
    EXPECT_EQ(c.serialiseStr(), example.code) << "input: " << example.code;
  }
}

TEST(CodeRoundTrip, QuadExamples)
{
  struct Example {
      uint16_t index;
      IrProto protocol;
      std::string code;
  };

  const std::vector<Example> examples = {
    { 3, protoBelgacom(), "0x0300F4010100010022C026F7020122C026F70222C0A6F7" },
    { 6, protoBeoDual(), "0x0600F401010200155594914001000102155589514000" },
    { 5, makeProto( { makeSection(17, 4) }), "0x0500F40101010015558555400000" },
    { 5, makeProto( { makeSection(17, 4) }), "0x0500F40101000100155591864000" },
    { 4, protoGalaxis(), "0x0400F4010100030022C00100029A0C00" },
    { 7, makeProto( { makeSection(14, 4) }), "0x07006400030001008899999000" },
    { 8, protoSingle(20), "0x0800F4010100010090B47000" },
  };

  for (const auto &example : examples) {
    Code c(example.index, example.protocol);
    ASSERT_EQ(c.parse(example.code), Status::OK) << "input: " << example.code;
    EXPECT_EQ(c.serialiseStr(), example.code) << "input: " << example.code;
  }
}

TEST(CodeRoundTrip, VectorFormRoundTrip)
{
  // serialiseVec() must match parse(vector<uint8_t>) the same way
  // serialiseStr() matches parse(string)
  const std::string original = "0x0200F4010300030070010002B93600";
  Code c(2, protoRc6());
  ASSERT_EQ(c.parse(original), Status::OK);

  auto bytes = c.serialiseVec();
  Code c2(2, protoRc6());
  ASSERT_EQ(c2.parse(bytes), Status::OK);
  EXPECT_EQ(c2.serialiseStr(), original);
}

TEST(CodeRoundTrip, VectorContent)
{
  Code c(0, protoNec(), "0x0000F40101010020DF32CD010100");
  const std::vector<uint8_t> expected = { 0x00, 0x00, 0xF4, 0x01, 0x01, 0x01, 0x00, 0x20, 0xDF, 0x32, 0xCD, 0x01, 0x01, 0x00 };
  EXPECT_EQ(c.serialiseVec(), expected);
}

// ===========================================================================
// Code - serialise from setters
// ===========================================================================

TEST(CodeSerialise, EmptyCode)
{
  Code c(0, protoNec());
  c.setDelay(500);
  c.setRepeats(1);
  EXPECT_EQ(c.serialiseStr(), "0x0000F40101000000");
}

TEST(CodeSerialise, DefaultValues)
{
  Code c(3, protoNec());
  EXPECT_EQ(c.getIndex(), 3);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 3);
  EXPECT_EQ(c.serialiseStr(), "0x0300F40103000000");
}

TEST(CodeSerialise, ResetRestoresDefaults)
{
  Code c(0, protoNec(), "0x0000C8000101004B407B84010100");
  c.reset();
  EXPECT_EQ(c.getIndex(), 0);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 3);
  EXPECT_TRUE(c.accessStartGroups().empty());
  EXPECT_TRUE(c.accessRepeatGroups().empty());
  EXPECT_TRUE(c.accessFinishGroups().empty());
}

TEST(CodeSerialise, SettersDelayRepeats)
{
  Code c(0, protoNec());
  c.setDelay(1500);
  c.setRepeats(2);
  EXPECT_EQ(c.getDelay(), 1500);
  EXPECT_EQ(c.getRepeats(), 2);
  EXPECT_EQ(c.serialiseStr(), "0x0000DC0502000000");
}

TEST(CodeSerialise, BelgacomFromSymbols)
{
  Code c(3, protoBelgacom());
  c.setDelay(500);
  c.setRepeats(1);
  c.setRepeatGroups( { { 0, { 0, 2, 0, 2, 3, 0, 0, 0, 0, 2, 1, 2, 3, 3, 1, 3 } } });
  c.setFinishGroups( { { 1, { 0, 2, 0, 2, 3, 0, 0, 0, 0, 2, 1, 2, 3, 3, 1, 3 } },
      { 2, { 0, 2, 0, 2, 3, 0, 0, 0, 2, 2, 1, 2, 3, 3, 1, 3 } } });
  EXPECT_EQ(c.serialiseStr(), "0x0300F4010100010022C026F7020122C026F70222C0A6F7");
}

TEST(CodeSerialise, BeoDualFromSymbols)
{
  Code c(6, protoBeoDual());
  c.setDelay(500);
  c.setRepeats(1);
  c.setStartGroups( { { 0, { 0, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 2, 1, 0, 1, 1 } }, { 1, { 0 } } });
  c.setRepeatGroups( { { 2, { 0, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 1, 1, 1, 0, 1, 1 } } });
  EXPECT_EQ(c.serialiseStr(), "0x0600F401010200155594914001000102155589514000");
}

TEST(CodeSerialise, ArtemideFromSymbols)
{
  Code c(7, makeProto( { makeSection(14, 4) }));
  c.setDelay(100);
  c.setRepeats(3);
  c.setRepeatGroups( { { 0, { 2, 0, 2, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 } } });
  EXPECT_EQ(c.serialiseStr(), "0x07006400030001008899999000");
}

TEST(CodeSerialise, GalaxisFromSymbols)
{
  Code c(4, protoGalaxis());
  c.setDelay(500);
  c.setRepeats(1);
  c.setRepeatGroups( { { 0, { 0, 2, 0, 2, 3, 0, 0, 0 } }, { 1, { 0 } }, { 2, { 2, 1, 2, 2, 0, 0, 3 } } });
  EXPECT_EQ(c.serialiseStr(), "0x0400F4010100030022C00100029A0C00");
}

TEST(CodeSerialise, GroupsReplaced)
{
  Code c(0, protoNec());
  c.setStartGroups( { { 0, Code::u64bitsToList(32, 0x20DF32CDULL) } });
  c.setStartGroups( { { 0, Code::u64bitsToList(32, 0x4B407B84ULL) } });
  ASSERT_EQ(c.accessStartGroups().size(), 1u);
  EXPECT_EQ(Code::listToU64bits(c.accessStartGroups()[0].getData()), 0x4B407B84ULL);
}

TEST(CodeSerialise, SetGroupUsesBitsOfProtocol)
{
  Code c(3, protoBelgacom());
  c.setRepeatGroups( { { 0, { 0, 2, 0, 2, 3, 0, 0, 0, 0, 2, 1, 2, 3, 3, 1, 3 } } });
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessRepeatGroups()[0].getBits(), 2);
  EXPECT_EQ(c.accessRepeatGroups()[0].getSection(), 0);
}

// ===========================================================================
// Code - create* / getData()
// ===========================================================================

TEST(CodeCreate, SimpleRepeatEqDataSamsung)
{
  auto c = Code::createSimpleRepeatEqData(1, protoSingle(32), 500, 32, 0x5F5F7788ULL, 3);
  EXPECT_EQ(c.getIndex(), 1);
  EXPECT_EQ(c.getDelay(), 500);
  EXPECT_EQ(c.getRepeats(), 3);
  EXPECT_EQ(c.serialiseStr(), "0x0100F401030001005F5F778800");
}

TEST(CodeCreate, SimpleRepeatEqDataDefaultRepeats)
{
  auto c = Code::createSimpleRepeatEqData(1, protoSingle(32), 500, 32, 0x5F5F7788ULL);
  EXPECT_EQ(c.getRepeats(), 3);
  EXPECT_EQ(c.accessStartGroups().size(), 0u);
  EXPECT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessFinishGroups().size(), 0u);
}

TEST(CodeCreate, SimpleRepeatEmptyLG)
{
  auto c = Code::createSimpleRepeatEmpty(0, protoNec(), 500, 32, 0x20DF32CDULL, 1);
  EXPECT_EQ(c.getRepeats(), 1);
  ASSERT_EQ(c.accessStartGroups().size(), 1u);
  ASSERT_EQ(c.accessRepeatGroups().size(), 1u);
  EXPECT_EQ(c.accessStartGroups()[0].getSection(), 0);
  EXPECT_EQ(c.accessRepeatGroups()[0].getSection(), 1);
  EXPECT_TRUE(c.accessRepeatGroups()[0].getData().empty());
  EXPECT_EQ(c.serialiseStr(), "0x0000F40101010020DF32CD010100");
}

TEST(CodeCreate, MultiRepeatEqDataPhilips)
{
  auto c = Code::createMultiRepeatEqData(2, protoRc6(), 500, { { 4, 0x7 }, { 1, 0x0 }, { 16, 0xB9FF } }, 3);
  EXPECT_EQ(c.accessRepeatGroups().size(), 3u);
  EXPECT_EQ(c.serialiseStr(), "0x0200F4010300030070010002B9FF00");
}

TEST(CodeCreate, FlatDefaultTxRoundTripsThroughGetData)
{
  // default repeats of the empty repeat frame variant: getData() must report
  // exactly the symbols that were set, once.
  auto c = Code::createSimpleRepeatEmpty(0, protoNec(), 500, 32, 0x20DF18E7ULL);
  auto data = c.getData();
  ASSERT_GE(data.size(), 1u); //c has two sections. payload in 0, repeat in 1. data[1] can be either empty container or missing entirely
  EXPECT_EQ(data[0].second.size(), 32u);
}

TEST(CodeCreate, FlatTxCountMatchesGetDataSize)
{
  // repeats=3, one 8 symbol frame in the repeat group
  Code c(0, protoSingle(8));
  ASSERT_EQ(c.parse("0x0000F40103000100AA00"), Status::OK);

  auto data = c.getData();
  EXPECT_EQ(data.size(), 3u);
}

TEST(CodeCreate, SingleSectionRoundTripsThroughSerialise)
{
  auto c = Code::createSimpleRepeatEqData(1, protoSingle(32), 500, 32, 0x5F5F7788ULL, 1);
  auto vec = c.serialiseVec();

  Code c2(1, protoSingle(32));
  ASSERT_EQ(c2.parse(vec), Status::OK);
  EXPECT_EQ(c2.getIndex(), 1);
  EXPECT_EQ(c2.accessRepeatGroups().size(), 1u);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 1u);
  ASSERT_EQ(data[0].second.size(), 32u);
  EXPECT_EQ(Code::listToU64bits(data[0].second), 0x5F5F7788ULL);
}

TEST(CodeCreate, MultiSectionRoundTripsThroughSerialise)
{
  auto c = Code::createMultiRepeatEqData(2, protoMulti16(), 500, {
    { 16, 0x7001 },
    { 16, 0x0002 },
    { 16, 0xB9FF },
  }, 1);
  auto vec = c.serialiseVec();

  Code c2(2, protoMulti16());
  ASSERT_EQ(c2.parse(vec), Status::OK);
  EXPECT_EQ(c2.getIndex(), 2);
  EXPECT_EQ(c2.accessRepeatGroups().size(), 3u);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 3u);
  EXPECT_EQ(data[0].second.size(), 16u);
  EXPECT_EQ(Code::listToU64bits(data[0].second), 0x7001ULL);
  EXPECT_EQ(Code::listToU64bits(data[1].second), 0x0002ULL);
  EXPECT_EQ(Code::listToU64bits(data[2].second), 0xB9FFULL);
}

TEST(CodeCreate, GetDataRepeatsMultiplyRepeatGroup)
{
  auto c = Code::createMultiRepeatEqData(2, protoMulti16(), 500, { { 16, 1 }, { 16, 2 }, { 16, 3 } }, 2);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 6u);
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[1].first, 1);
  EXPECT_EQ(data[2].first, 2);
  EXPECT_EQ(data[3].first, 0);
  EXPECT_EQ(data[4].first, 1);
  EXPECT_EQ(data[5].first, 2);
}

TEST(CodeCreate, GetDataZeroRepeats)
{
  auto c = Code::createSimpleRepeatEmpty(0, protoNec(), 500, 32, 0x20DF32CDULL, 0);
  auto data = c.getData();
  ASSERT_EQ(data.size(), 1u);
  EXPECT_EQ(data[0].first, 0);
}

// ===========================================================================
// Code - bit packing (round trip via serialise, parse and getData)
// ===========================================================================

TEST(CodeBitPacking, NonByteAlignedBitCountPadsWithZeros)
{
  // 12 bits: 1010 1100 1101 -> 0xAC, 0xD0 (last nibble zero-padded)
  auto c = Code::createSimpleRepeatEqData(0, protoSingle(12), 500, 12, 0xACDULL, 1);
  auto vec = c.serialiseVec();

  const std::vector<uint8_t> expectedPayload = { 0xAC, 0xD0 };
  ASSERT_EQ(vec.size(), 5u + 1u + 1u + 1u + 2u + 1u);
  EXPECT_EQ(std::vector<uint8_t>(vec.begin() + 8, vec.begin() + 10), expectedPayload);

  Code c2(0, protoSingle(12));
  ASSERT_EQ(c2.parse(vec), Status::OK);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 1u);
  // padding is not a symbol
  EXPECT_EQ(data[0].second.size(), 12u);
  EXPECT_EQ(Code::listToU64bits(data[0].second), 0xACDULL);
}

TEST(CodeBitPacking, SixtyFourBitPayload)
{
  auto c = Code::createSimpleRepeatEqData(0, protoSingle(64), 500, 64, 0xFFEEDDCCBBAA9988ULL, 1);
  auto vec = c.serialiseVec();

  Code c2(0, protoSingle(64));
  ASSERT_EQ(c2.parse(vec), Status::OK);
  auto data = c2.getData();
  ASSERT_EQ(data.size(), 1u);
  ASSERT_EQ(data[0].second.size(), 64u);
  // 0xFF = 11111111
  EXPECT_EQ(data[0].second[0], 1);
  EXPECT_EQ(data[0].second[7], 1);
  // last byte 0x88 = 10001000
  EXPECT_EQ(data[0].second[56], 1);
  EXPECT_EQ(data[0].second[57], 0);
  EXPECT_EQ(Code::listToU64bits(data[0].second), 0xFFEEDDCCBBAA9988ULL);
}

TEST(CodeBitPacking, QuadPayloadPadsWithZeros)
{
  // 7 quad symbols = 14 bits -> 9A 0C, low 2 bits are padding
  Code c(4, protoGalaxis());
  c.setRepeats(1);
  c.setRepeatGroups( { { 2, { 2, 1, 2, 2, 0, 0, 3 } } });
  auto vec = c.serialiseVec();
  ASSERT_EQ(vec.size(), 5u + 1u + 1u + 1u + 2u + 1u);
  EXPECT_EQ(vec[8], 0x9A);
  EXPECT_EQ(vec[9], 0x0C);
}
