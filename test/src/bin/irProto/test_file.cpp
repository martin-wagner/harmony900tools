// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for irProto::File, IrProto, TimingSection,
 * TimingSectionIrHeader and TimingSectionIrPayload.
 *
 * Real IrProto.bin structure (verified by hex inspection):
 *   8 protocols, CRC32=0x74693195, data size=455 bytes
 *
 * Protocols without a real file are built in the fixtures below from the
 * timings of the protocol definitions:
 *   NEC style (Toshiba 32 Bit): data section + repeat frame, 107ms interval
 *   Belgacom 16 Bit Quad: 3 sections, 4 states per symbol
 *
 * Device command examples extracted from UserConfiguration.xml
 * (one per physical device, not per protocol):
 *
 *   LG    82UN85006LA   proto=0  0x0000F40101010020DF32CD010100
 *   Samsung SV-6332X    proto=1  0x0100F401030001005F5F778800
 *   Philips BDP-2700    proto=2  0x0200F4010300030070010002B9FF00
 *   Onkyo TX-NR414      proto=0  0x0000C8000101004B407B84010100
 *   Vu+ DUO 2           proto=4  0x0400C80001000100FEB5BFFC00
 *   Microsoft XBox      proto=5  0x0500F40103000100530ACF00
 *   Led RGB-LED-1       proto=6  0x0600F401030001009C0000
 *   Belgacom ISB6030    proto=3  0x0300F4010100010022C026F7020122C026F70222C0A6F7
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <string>
#include <vector>

#include "file.h"
#include "code.h"

#include "lib/harmony_endian.h"
#include "lib/crc32.h"

using namespace binary;
using namespace irProto;

#ifndef REAL_IRPROTO_BIN_PATH
#define REAL_IRPROTO_BIN_PATH "IrProto.bin"
#endif

namespace {

using Quantum = std::vector<Item>;

TimingSectionIrHeader makeHeader(const std::vector<Item> &items)
{
  TimingSectionIrHeader header;

  for (const auto &item : items) {
    header.addItem(item);
  }
  return header;
}

TimingSectionIrPayload makePayload(const std::vector<Quantum> &quanta)
{
  TimingSectionIrPayload payload;

  for (size_t i = 0; i < quanta.size(); i++) {
    Quantum quantum = quanta[i];
    payload.setQuantum(static_cast<uint8_t>(i), quantum);
  }
  return payload;
}

TimingSection makeSection(uint16_t bitCount, uint32_t interval,
    const TimingSectionIrHeader &sof, const TimingSectionIrPayload &payload,
    const TimingSectionIrHeader &eof, uint16_t togglePos =
        TimingSection::TOGGLE_NONE)
{
  TimingSection section;

  section.setBitCount(bitCount);
  section.setToggle(togglePos);
  section.setTiming(interval);
  section.setSoF(sof);
  section.setData(payload);
  section.setEoF(eof);
  return section;
}

// Toshiba 32 Bit: 32 symbol data frame (107ms), empty repeat frame
IrProto protoNec()
{
  auto data = makeSection(32, 107000, makeHeader( { { true, 8990 }, { false,
      4490 } }), makePayload( { { { true, 568 }, { false, 552 } }, { { true,
      568 }, { false, 1662 } } }), makeHeader( { { true, 568 } }));
  auto repeat = makeSection(0, TimingSection::PAUSE_IN_EOF, makeHeader( { {
      true, 8990 }, { false, 2230 } }), TimingSectionIrPayload(), makeHeader( {
      { true, 568 }, { false, 32767 }, { false, 32767 }, { false, 30543 } }));
  return IrProto(26315, { data, repeat });
}

// Belgacom 16 Bit Quad: 3 frames of 16 symbols with 4 states, different EoF
IrProto protoBelgacom()
{
  auto payload = makePayload( { { { true, 158 }, { false, 286 } }, { { true,
      158 }, { false, 453 } }, { { true, 158 }, { false, 620 } }, { { true,
      158 }, { false, 786 } } });
  auto sof = makeHeader( { { true, 418 }, { false, 286 } });

  auto s0 = makeSection(16, TimingSection::PAUSE_IN_EOF, sof, payload,
      makeHeader( { { true, 160 }, { false, 32767 }, { false, 32767 }, { false,
          24766 } }));
  auto s1 = makeSection(16, TimingSection::PAUSE_IN_EOF, sof, payload,
      makeHeader( { { true, 158 }, { false, 20065 }, { false, 20066 } }));
  auto s2 = makeSection(16, TimingSection::PAUSE_IN_EOF, sof, payload,
      makeHeader( { { true, 158 } }));
  return IrProto(27777, { s0, s1, s2 });
}

// 2 symbol frame, no interval
TimingSection sectionTiny(uint32_t interval, uint16_t bitCount = 4)
{
  return makeSection(bitCount, interval, makeHeader( { { true, 9000 }, { false,
      4500 } }), makePayload( { { { true, 560 }, { false, 560 } }, { { true,
      560 }, { false, 1680 } } }), makeHeader( { { true, 560 } }));
}

std::vector<uint8_t> readFile(const std::string &path)
{
  std::ifstream in(path, std::ios::binary);
  return std::vector<uint8_t>(std::istreambuf_iterator<char>(in),
      std::istreambuf_iterator<char>());
}

void writeFile(const std::string &path, const std::vector<uint8_t> &data)
{
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char*>(data.data()), data.size());
}

std::string tempPath(const std::string &name)
{
  return (std::filesystem::temp_directory_path() / name).string();
}

// recalculate size and crc of the file wrapper after the test modified the payload
std::vector<uint8_t> fixWrapper(std::vector<uint8_t> raw)
{
  std::vector<uint8_t> payload(raw.begin() + 8, raw.end());
  std::vector<uint8_t> wrapper;

  lib::setHarmony32_file(lib::calcCrc32(payload), wrapper);
  lib::setHarmony16_file(payload.size(), wrapper);
  lib::setHarmony16_file(0, wrapper);
  std::copy(wrapper.begin(), wrapper.end(), raw.begin());
  return raw;
}

Status parseBytes(const std::vector<uint8_t> &raw, File &file)
{
  auto path = tempPath("irproto_test_parse.bin");

  writeFile(path, raw);
  auto status = file.parse(path);
  std::filesystem::remove(path);
  return status;
}

void expectPair(TimingStream &stream, size_t index, unsigned mark,
    unsigned pause)
{
  ASSERT_LT(index, stream.timings().size());
  EXPECT_EQ(stream.timings()[index].mark_us, mark) << "index " << index;
  EXPECT_EQ(stream.timings()[index].pause_us, pause) << "index " << index;
}

}

// ===========================================================================
// TimingSectionIrHeader
// ===========================================================================

TEST(TimingSectionIrHeader, DefaultEmpty)
{
  TimingSectionIrHeader h;
  EXPECT_TRUE(h.isEmpty());
  EXPECT_EQ(h.getCount(), 0);
}

TEST(TimingSectionIrHeader, AddItemMark)
{
  TimingSectionIrHeader h;
  h.addItem( { true, 8990 });
  ASSERT_EQ(h.getCount(), 1);
  EXPECT_EQ(h.accessStream()[0].first, true);
  EXPECT_EQ(h.accessStream()[0].second, 8990u);
}

TEST(TimingSectionIrHeader, AddItemPause)
{
  TimingSectionIrHeader h;
  h.addItem( { false, 4490 });
  ASSERT_EQ(h.getCount(), 1);
  EXPECT_EQ(h.accessStream()[0].first, false);
  EXPECT_EQ(h.accessStream()[0].second, 4490u);
}

TEST(TimingSectionIrHeader, AddItemCropsOverflow)
{
  // values >= 0x8000 must be cropped to 0x7fff
  TimingSectionIrHeader h;
  h.addItem( { true, 0x9000 });
  EXPECT_EQ(h.accessStream()[0].second, 0x7fffu);
}

TEST(TimingSectionIrHeader, SerialiseRoundTrip)
{
  TimingSectionIrHeader a;
  a.addItem( { true, 8990 });
  a.addItem( { false, 4490 });

  auto bytes = a.serialise();
  // size byte + 2 items * 2 bytes = 5
  ASSERT_EQ(bytes.size(), 5u);
  EXPECT_EQ(bytes[0], 2u); // item count

  TimingSectionIrHeader b(bytes);
  ASSERT_EQ(b.getCount(), 2);
  EXPECT_EQ(b.accessStream()[0].first, true);
  EXPECT_EQ(b.accessStream()[0].second, 8990u);
  EXPECT_EQ(b.accessStream()[1].first, false);
  EXPECT_EQ(b.accessStream()[1].second, 4490u);
}

TEST(TimingSectionIrHeader, SerialiseEmptyReturnsEmpty)
{
  TimingSectionIrHeader h;
  EXPECT_TRUE(h.serialise().empty());
}

TEST(TimingSectionIrHeader, SerialiseIrStream)
{
  TimingSectionIrHeader h;
  h.addItem( { true, 425 });
  h.addItem( { false, 32767 });

  std::vector<Item> out;
  h.serialiseIrStream(out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0], Item(true, 425));
  EXPECT_EQ(out[1], Item(false, 32767));
}

TEST(TimingSectionIrHeader, SerialiseIrStreamAppends)
{
  TimingSectionIrHeader h;
  h.addItem( { true, 425 });

  std::vector<Item> out = { { false, 100 } };
  h.serialiseIrStream(out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0], Item(false, 100));
  EXPECT_EQ(out[1], Item(true, 425));
}

TEST(TimingSectionIrHeader, SerialiseBytes)
{
  // mark 425 = 0x81A9, pause 450 = 0x01C2, little endian
  auto bytes = makeHeader( { { true, 425 }, { false, 450 } }).serialise();
  const std::vector<uint8_t> expected = { 0x02, 0xA9, 0x81, 0xC2, 0x01 };
  EXPECT_EQ(bytes, expected);
}

TEST(TimingSectionIrHeader, ParseEofWithInterFrameGap)
{
  // EoF with included inter-frame gap: mark 425, pause 32767, pause 21016, pause 21017
  TimingSectionIrHeader h( { 0x04, 0xA9, 0x81, 0xFF, 0x7F, 0x18, 0x52, 0x19,
      0x52 });
  ASSERT_EQ(h.getCount(), 4);
  EXPECT_EQ(h.accessStream()[0], Item(true, 425));
  EXPECT_EQ(h.accessStream()[1], Item(false, 32767));
  EXPECT_EQ(h.accessStream()[2], Item(false, 21016));
  EXPECT_EQ(h.accessStream()[3], Item(false, 21017));
}

TEST(TimingSectionIrHeader, ParseEmptyInput)
{
  TimingSectionIrHeader h(std::vector<uint8_t>( { }));
  EXPECT_TRUE(h.isEmpty());
}

TEST(TimingSectionIrHeader, ParseOddByteCountRejected)
{
  TimingSectionIrHeader h( { 0x01, 0xA9, 0x81, 0x00 });
  EXPECT_TRUE(h.isEmpty());
}

TEST(TimingSectionIrHeader, ParseTooFewItemsRejected)
{
  // announces 3 items, only 2 given
  TimingSectionIrHeader h( { 0x03, 0xA9, 0x81, 0xC2, 0x01 });
  EXPECT_TRUE(h.isEmpty());
}

TEST(TimingSectionIrHeader, ParseReadsAnnouncedItemsOnly)
{
  // announces 1 item, 2 given: the item count is authoritative
  TimingSectionIrHeader h( { 0x01, 0xA9, 0x81, 0xC2, 0x01 });
  ASSERT_EQ(h.getCount(), 1);
  EXPECT_EQ(h.accessStream()[0], Item(true, 425));
}

// ===========================================================================
// TimingSectionIrPayload
// ===========================================================================

TEST(TimingSectionIrPayload, DefaultEmpty)
{
  TimingSectionIrPayload p;
  EXPECT_TRUE(p.isEmpty());
  EXPECT_EQ(p.getQuantisation(), 0u);
  EXPECT_EQ(p.getTperQantum(), 0u);
  EXPECT_FALSE(p.isBoolean());
  EXPECT_FALSE(p.isQuad());
  EXPECT_FALSE(p.isHex());
}

TEST(TimingSectionIrPayload, SetQuantumRejectsEmptyTimings)
{
  TimingSectionIrPayload p;
  Quantum q;
  EXPECT_FALSE(p.setQuantum(0, q));
  EXPECT_TRUE(p.isEmpty());
}

TEST(TimingSectionIrPayload, SetQuantumFirstSetsTimingCount)
{
  TimingSectionIrPayload p;
  Quantum q = { { true, 568 }, { false, 552 } };
  EXPECT_TRUE(p.setQuantum(0, q));
  EXPECT_FALSE(p.isEmpty());
  EXPECT_EQ(p.getQuantisation(), 1u);
  EXPECT_EQ(p.getTperQantum(), 2u);
}

TEST(TimingSectionIrPayload, SetQuantumRejectsDifferentTimingCount)
{
  // all quanta need the same timing count as the first one
  TimingSectionIrPayload p;
  Quantum q0 = { { true, 500 }, { false, 500 } };
  Quantum q1 = { { true, 500 } };
  ASSERT_TRUE(p.setQuantum(0, q0));
  EXPECT_FALSE(p.setQuantum(1, q1));
  EXPECT_EQ(p.getQuantisation(), 1u);
}

TEST(TimingSectionIrPayload, SetQuantumRejectsGap)
{
  // positions need to be added in order
  TimingSectionIrPayload p;
  Quantum q = { { true, 500 }, { false, 500 } };
  EXPECT_FALSE(p.setQuantum(1, q));
  EXPECT_TRUE(p.isEmpty());

  ASSERT_TRUE(p.setQuantum(0, q));
  EXPECT_FALSE(p.setQuantum(2, q));
  EXPECT_EQ(p.getQuantisation(), 1u);
}

TEST(TimingSectionIrPayload, SetQuantumReplacesExisting)
{
  TimingSectionIrPayload p;
  Quantum q0 = { { true, 500 }, { false, 500 } };
  Quantum q1 = { { true, 600 }, { false, 700 } };
  ASSERT_TRUE(p.setQuantum(0, q0));
  ASSERT_TRUE(p.setQuantum(0, q1));
  EXPECT_EQ(p.getQuantisation(), 1u);
  EXPECT_EQ(p.accessQuantum(0)[0], Item(true, 600));
  EXPECT_EQ(p.accessQuantum(0)[1], Item(false, 700));
}

TEST(TimingSectionIrPayload, SetAndAccessQuanta)
{
  TimingSectionIrPayload p;
  Quantum falseQuantum = { { true, 568 }, { false, 552 } };
  Quantum trueQuantum = { { true, 568 }, { false, 1662 } };
  ASSERT_TRUE(p.setQuantum(TimingSectionIrPayload::QUANTUM_FALSE, falseQuantum));
  ASSERT_TRUE(p.setQuantum(TimingSectionIrPayload::QUANTUM_TRUE, trueQuantum));

  ASSERT_EQ(p.accessQuantum(0).size(), 2u);
  ASSERT_EQ(p.accessQuantum(1).size(), 2u);

  EXPECT_EQ(p.accessQuantum(0)[0], Item(true, 568));
  EXPECT_EQ(p.accessQuantum(0)[1], Item(false, 552));
  EXPECT_EQ(p.accessQuantum(1)[0], Item(true, 568));
  EXPECT_EQ(p.accessQuantum(1)[1], Item(false, 1662));
}

TEST(TimingSectionIrPayload, AccessOutOfRangeIsEmpty)
{
  TimingSectionIrPayload p;
  Quantum q = { { true, 568 }, { false, 552 } };
  ASSERT_TRUE(p.setQuantum(0, q));
  EXPECT_TRUE(p.accessQuantum(1).empty());
  EXPECT_TRUE(p.accessQuantum(255).empty());
}

TEST(TimingSectionIrPayload, QuantisationKinds)
{
  Quantum q = { { true, 500 }, { false, 500 } };

  TimingSectionIrPayload two;
  for (uint8_t i = 0; i < 2; i++) {
    ASSERT_TRUE(two.setQuantum(i, q));
  }
  EXPECT_TRUE(two.isBoolean());
  EXPECT_FALSE(two.isQuad());
  EXPECT_FALSE(two.isHex());

  TimingSectionIrPayload four;
  for (uint8_t i = 0; i < 4; i++) {
    ASSERT_TRUE(four.setQuantum(i, q));
  }
  EXPECT_FALSE(four.isBoolean());
  EXPECT_TRUE(four.isQuad());
  EXPECT_FALSE(four.isHex());
  EXPECT_EQ(four.getQuantisation(), 4u);

  TimingSectionIrPayload sixteen;
  for (uint8_t i = 0; i < 16; i++) {
    ASSERT_TRUE(sixteen.setQuantum(i, q));
  }
  EXPECT_FALSE(sixteen.isBoolean());
  EXPECT_FALSE(sixteen.isQuad());
  EXPECT_TRUE(sixteen.isHex());
}

TEST(TimingSectionIrPayload, SerialiseIrStreamFalse)
{
  auto p = makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
      false, 1662 } } });

  std::vector<Item> out;
  p.serialiseIrStream(0, out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0], Item(true, 568));
  EXPECT_EQ(out[1], Item(false, 552));
}

TEST(TimingSectionIrPayload, SerialiseIrStreamTrue)
{
  auto p = makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
      false, 1662 } } });

  std::vector<Item> out;
  p.serialiseIrStream(1, out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[1], Item(false, 1662));
}

TEST(TimingSectionIrPayload, SerialiseIrStreamQuad)
{
  // Belgacom 16 Bit Quad, symbol 3
  std::vector<Item> out;
  auto p = makePayload( { { { true, 158 }, { false, 286 } }, { { true, 158 }, {
      false, 453 } }, { { true, 158 }, { false, 620 } }, { { true, 158 }, {
      false, 786 } } });
  p.serialiseIrStream(3, out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[1], Item(false, 786));
}

TEST(TimingSectionIrPayload, SerialiseIrStreamUnknownState)
{
  auto p = makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
      false, 1662 } } });

  std::vector<Item> out;
  p.serialiseIrStream(7, out);
  EXPECT_TRUE(out.empty());
}

TEST(TimingSectionIrPayload, SerialiseBytes)
{
  // mark 568 = 0x8238, pause 552 = 0x0228, mark 568, pause 1662 = 0x067E
  auto p = makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
      false, 1662 } } });
  const std::vector<uint8_t> expected = { 0x38, 0x82, 0x28, 0x02, 0x38, 0x82,
      0x7E, 0x06 };
  EXPECT_EQ(p.serialise(), expected);
}

TEST(TimingSectionIrPayload, SerialiseEmpty)
{
  TimingSectionIrPayload p;
  EXPECT_TRUE(p.serialise().empty());
}

TEST(TimingSectionIrPayload, ParseFromBytes)
{
  // two quanta, two timings each: (mark 425, pause 450) (mark 425, pause 1320)
  TimingSectionIrPayload p(2, 2, { 0xA9, 0x81, 0xC2, 0x01, 0xA9, 0x81, 0x28,
      0x05 });
  ASSERT_EQ(p.getQuantisation(), 2u);
  EXPECT_EQ(p.getTperQantum(), 2u);
  EXPECT_TRUE(p.isBoolean());
  EXPECT_EQ(p.accessQuantum(0)[0], Item(true, 425));
  EXPECT_EQ(p.accessQuantum(0)[1], Item(false, 450));
  EXPECT_EQ(p.accessQuantum(1)[0], Item(true, 425));
  EXPECT_EQ(p.accessQuantum(1)[1], Item(false, 1320));
}

TEST(TimingSectionIrPayload, ParseSerialiseRoundTrip)
{
  auto p = makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
      false, 1662 } } });
  TimingSectionIrPayload q(2, 2, p.serialise());
  ASSERT_EQ(q.getQuantisation(), 2u);
  EXPECT_EQ(q.accessQuantum(0), p.accessQuantum(0));
  EXPECT_EQ(q.accessQuantum(1), p.accessQuantum(1));
}

TEST(TimingSectionIrPayload, ParseEmptyData)
{
  TimingSectionIrPayload p(2, 2, { });
  EXPECT_TRUE(p.isEmpty());
}

TEST(TimingSectionIrPayload, ParseOddByteCountRejected)
{
  TimingSectionIrPayload p(1, 2, { 0xA9, 0x81, 0xC2 });
  EXPECT_TRUE(p.isEmpty());
}

TEST(TimingSectionIrPayload, ParseWrongTimingCountRejected)
{
  // 2 quanta * 2 timings = 8 bytes needed
  TimingSectionIrPayload p(2, 2, { 0xA9, 0x81, 0xC2, 0x01, 0xA9, 0x81 });
  EXPECT_TRUE(p.isEmpty());
}

TEST(TimingSectionIrPayload, ParseZeroQuantaRejected)
{
  TimingSectionIrPayload p(0, 2, { 0xA9, 0x81, 0xC2, 0x01 });
  EXPECT_TRUE(p.isEmpty());
}

// ===========================================================================
// TimingSection
// ===========================================================================

TEST(TimingSection, DefaultValues)
{
  TimingSection s;
  EXPECT_EQ(s.getBitCount(), 0u);
  EXPECT_FALSE(s.hasToggle());
  EXPECT_EQ(s.getToggle(), TimingSection::TOGGLE_NONE);
  EXPECT_EQ(s.getTiming(), TimingSection::PAUSE_IN_EOF);
  EXPECT_TRUE(s.getSoF().isEmpty());
  EXPECT_TRUE(s.getData().isEmpty());
  EXPECT_TRUE(s.getEoF().isEmpty());
}

TEST(TimingSection, Setters)
{
  TimingSection s;
  s.setBitCount(48);
  s.setToggle(14);
  s.setTiming(107000);
  EXPECT_EQ(s.getBitCount(), 48u);
  EXPECT_TRUE(s.hasToggle());
  EXPECT_EQ(s.getToggle(), 14u);
  EXPECT_EQ(s.getTiming(), 107000u);

  s.setToggle();
  s.setTiming();
  EXPECT_FALSE(s.hasToggle());
  EXPECT_EQ(s.getTiming(), TimingSection::PAUSE_IN_EOF);
}

TEST(TimingSection, SetTimingSections)
{
  auto s = makeSection(32, 107000, makeHeader( { { true, 8990 }, { false, 4490 } }),
      makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
          false, 1662 } } }), makeHeader( { { true, 568 } }));
  EXPECT_EQ(s.getSoF().getCount(), 2);
  EXPECT_EQ(s.getEoF().getCount(), 1);
  EXPECT_TRUE(s.getData().isBoolean());
}

TEST(TimingSection, SerialiseLayout)
{
  const int offset = 100;
  auto s = makeSection(32, 107000, makeHeader( { { true, 8990 }, { false, 4490 } }),
      makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
          false, 1662 } } }), makeHeader( { { true, 568 } }));
  auto bytes = s.serialise(offset);

  // header 16 bytes, SoF 5 bytes, EoF 3 bytes, payload 8 bytes
  ASSERT_EQ(bytes.size(), 16u + 5u + 3u + 8u);
  EXPECT_EQ(bytes[0], 32u);   // bit count
  EXPECT_EQ(bytes[1], 0u);
  EXPECT_EQ(bytes[2], 0xFFu); // toggle: none
  EXPECT_EQ(bytes[3], 0xFFu); // padding
  EXPECT_EQ(bytes[4], 0xF8u); // interval 107000 = 0x0001A1F8
  EXPECT_EQ(bytes[5], 0xA1u);
  EXPECT_EQ(bytes[6], 0x01u);
  EXPECT_EQ(bytes[7], 0x00u);
  EXPECT_EQ(bytes[8], 2u);    // quantum count
  EXPECT_EQ(bytes[9], 2u);    // timings per quantum
  // pointers: payload, SoF, EoF (absolute, including offset)
  EXPECT_EQ(bytes[10] | (bytes[11] << 8), offset + 16 + 5 + 3);
  EXPECT_EQ(bytes[12] | (bytes[13] << 8), offset + 16);
  EXPECT_EQ(bytes[14] | (bytes[15] << 8), offset + 16 + 5);
}

TEST(TimingSection, SerialiseRoundTrip)
{
  const int offset = 100;
  auto s = makeSection(32, 107000, makeHeader( { { true, 8990 }, { false, 4490 } }),
      makePayload( { { { true, 568 }, { false, 552 } }, { { true, 568 }, {
          false, 1662 } } }), makeHeader( { { true, 568 } }));
  TimingSection t(s.serialise(offset), offset);

  EXPECT_EQ(t.getBitCount(), 32u);
  EXPECT_FALSE(t.hasToggle());
  EXPECT_EQ(t.getTiming(), 107000u);
  ASSERT_EQ(t.getSoF().getCount(), 2);
  EXPECT_EQ(t.getSoF().accessStream()[0], Item(true, 8990));
  EXPECT_EQ(t.getSoF().accessStream()[1], Item(false, 4490));
  ASSERT_EQ(t.getEoF().getCount(), 1);
  EXPECT_EQ(t.getEoF().accessStream()[0], Item(true, 568));
  ASSERT_EQ(t.getData().getQuantisation(), 2u);
  EXPECT_EQ(t.getData().accessQuantum(0)[1], Item(false, 552));
  EXPECT_EQ(t.getData().accessQuantum(1)[1], Item(false, 1662));
  EXPECT_EQ(t.serialise(offset), s.serialise(offset));
}

TEST(TimingSection, SerialiseRoundTripToggle)
{
  auto s = makeSection(30, TimingSection::PAUSE_IN_EOF, makeHeader( { { true,
      2632 }, { false, 900 } }), makePayload( { { { true, 882 }, { false, 446 } },
      { { true, 446 }, { false, 882 } } }), makeHeader( { { true, 446 } }), 14);
  TimingSection t(s.serialise(0), 0);

  EXPECT_TRUE(t.hasToggle());
  EXPECT_EQ(t.getToggle(), 14u);
  EXPECT_EQ(t.getTiming(), TimingSection::PAUSE_IN_EOF);
  EXPECT_EQ(t.getBitCount(), 30u);
}

TEST(TimingSection, SerialiseRoundTripRepeatFrame)
{
  // no payload, EoF with 4 items
  auto s = makeSection(0, TimingSection::PAUSE_IN_EOF, makeHeader( { { true,
      8990 }, { false, 2230 } }), TimingSectionIrPayload(), makeHeader( { {
      true, 568 }, { false, 32767 }, { false, 32767 }, { false, 30543 } }));
  auto bytes = s.serialise(50);
  EXPECT_EQ(bytes[8], 0u);
  EXPECT_EQ(bytes[9], 0u);
  EXPECT_EQ(bytes[10], 0u); // no payload pointer
  EXPECT_EQ(bytes[11], 0u);

  TimingSection t(bytes, 50);
  EXPECT_EQ(t.getBitCount(), 0u);
  EXPECT_TRUE(t.getData().isEmpty());
  ASSERT_EQ(t.getSoF().getCount(), 2);
  ASSERT_EQ(t.getEoF().getCount(), 4);
  EXPECT_EQ(t.getEoF().accessStream()[3], Item(false, 30543));
}

TEST(TimingSection, ParseTooSmall)
{
  TimingSection t(std::vector<uint8_t>( { 0x20, 0x00, 0xFF, 0xFF, 0x00 }), 0);
  EXPECT_EQ(t.getBitCount(), 0u);
  EXPECT_TRUE(t.getSoF().isEmpty());
}

TEST(TimingSection, ParsePointerBehindDataIsIgnored)
{
  // header only, all pointers point right behind the data
  std::vector<uint8_t> bytes = { 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x02, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00 };
  TimingSection t(bytes, 0);
  EXPECT_EQ(t.getBitCount(), 32u);
  EXPECT_TRUE(t.getSoF().isEmpty());
  EXPECT_TRUE(t.getEoF().isEmpty());
  EXPECT_TRUE(t.getData().isEmpty());
}

TEST(TimingSection, ParsePointerOutsideDataIsIgnored)
{
  std::vector<uint8_t> bytes = { 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x02, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00 };
  TimingSection t(bytes, 0);
  EXPECT_TRUE(t.getSoF().isEmpty());
  EXPECT_TRUE(t.getEoF().isEmpty());
  EXPECT_TRUE(t.getData().isEmpty());
}

// ---------------------------------------------------------------------------
// timing stream of a single section
// ---------------------------------------------------------------------------

TEST(TimingSectionStream, FrameWithoutInterval)
{
  std::vector<Item> out;
  sectionTiny(TimingSection::PAUSE_IN_EOF).serialiseIrStream(out, { 0, 1, 1, 0 });

  // SoF (2) + 4 symbols * 2 + EoF (1)
  ASSERT_EQ(out.size(), 11u);
  EXPECT_EQ(out[0], Item(true, 9000));
  EXPECT_EQ(out[1], Item(false, 4500));
  EXPECT_EQ(out[3], Item(false, 560));  // symbol 0
  EXPECT_EQ(out[5], Item(false, 1680)); // symbol 1
  EXPECT_EQ(out[7], Item(false, 1680)); // symbol 2
  EXPECT_EQ(out[9], Item(false, 560));  // symbol 3
  EXPECT_EQ(out[10], Item(true, 560));
}

TEST(TimingSectionStream, IntervalFillsWithPauses)
{
  // frame is 20780us long, interval 107000us -> 86220us pause in 0x7fff chunks
  std::vector<Item> out;
  sectionTiny(107000).serialiseIrStream(out, { 0, 1, 1, 0 });

  ASSERT_EQ(out.size(), 14u);
  EXPECT_EQ(out[11], Item(false, 32767));
  EXPECT_EQ(out[12], Item(false, 32767));
  EXPECT_EQ(out[13], Item(false, 20686));

  uint32_t total = 0;
  for (const auto &item : out) {
    total += item.second;
  }
  EXPECT_EQ(total, 107000u);
}

TEST(TimingSectionStream, IntervalShorterThanFrame)
{
  // interval is shorter than the frame: fallback pause
  std::vector<Item> out;
  sectionTiny(10000).serialiseIrStream(out, { 0, 1, 1, 0 });

  ASSERT_EQ(out.size(), 13u);
  EXPECT_EQ(out[11], Item(false, 32767));
  EXPECT_EQ(out[12], Item(false, 17233));
}

TEST(TimingSectionStream, TooLittleDataIsIgnored)
{
  std::vector<Item> out;
  sectionTiny(TimingSection::PAUSE_IN_EOF).serialiseIrStream(out, { 0, 1, 1 });
  EXPECT_TRUE(out.empty());
}

TEST(TimingSectionStream, AppendsToOutput)
{
  std::vector<Item> out = { { true, 1 } };
  sectionTiny(TimingSection::PAUSE_IN_EOF, 1).serialiseIrStream(out, { 1 });
  ASSERT_EQ(out.size(), 6u);
  EXPECT_EQ(out[0], Item(true, 1));
  EXPECT_EQ(out[1], Item(true, 9000));
}

TEST(TimingSectionStream, SymbolBeyondStatesIsRejected)
{
  // boolean payload, symbol 2 does not exist
  std::vector<Item> out;
  sectionTiny(TimingSection::PAUSE_IN_EOF, 2).serialiseIrStream(out, { 0, 2 });
  EXPECT_TRUE(out.empty());
}

TEST(TimingSectionStream, EmptyRepeatFrame)
{
  auto s = makeSection(0, TimingSection::PAUSE_IN_EOF, makeHeader( { { true,
      8990 }, { false, 2230 } }), TimingSectionIrPayload(), makeHeader( { {
      true, 568 }, { false, 32767 }, { false, 32767 }, { false, 30543 } }));
  std::vector<Item> out;
  s.serialiseIrStream(out, { });
  ASSERT_EQ(out.size(), 6u);
  EXPECT_EQ(out[0], Item(true, 8990));
  EXPECT_EQ(out[1], Item(false, 2230));
  EXPECT_EQ(out[5], Item(false, 30543));
}

// ===========================================================================
// IrProto
// ===========================================================================

TEST(IrProto, DefaultEmpty)
{
  IrProto p;
  EXPECT_TRUE(p.isEmpty());
  EXPECT_EQ(p.getSectionCount(), 0);
  EXPECT_EQ(p.getBitCount(), 0);
}

TEST(IrProto, CreateFromSections)
{
  auto p = protoNec();
  EXPECT_FALSE(p.isEmpty());
  EXPECT_EQ(p.getSectionCount(), 2);
  EXPECT_EQ(p.getBitCount(), 32);
  EXPECT_EQ(p.getClockPeriod(), 26315u);
  EXPECT_NEAR(p.getClock(), 38000.0, 100.0);
}

TEST(IrProto, BitCountIsSumOfSections)
{
  EXPECT_EQ(protoBelgacom().getBitCount(), 48);
}

TEST(IrProto, AccessSection)
{
  auto p = protoBelgacom();
  EXPECT_EQ(p.accessSection(0).getEoF().getCount(), 4);
  EXPECT_EQ(p.accessSection(1).getEoF().getCount(), 3);
  EXPECT_EQ(p.accessSection(2).getEoF().getCount(), 1);
  EXPECT_NEAR(p.getClock(), 36001.0, 100.0);
}

TEST(IrProto, AccessSectionOutOfRangeIsEmpty)
{
  auto p = protoNec();
  EXPECT_EQ(p.accessSection(2).getBitCount(), 0u);
  EXPECT_TRUE(p.accessSection(2).getSoF().isEmpty());
  EXPECT_EQ(p.accessSection(-1).getBitCount(), 0u);
}

TEST(IrProto, SerialiseHeader)
{
  auto bytes = protoNec().serialise(0);
  ASSERT_GE(bytes.size(), 11u);
  EXPECT_EQ(bytes[0], 0x01u);  // start
  EXPECT_EQ(bytes[1], 0xCBu);  // clock period 26315 = 0x66CB
  EXPECT_EQ(bytes[2], 0x66u);
  EXPECT_EQ(bytes[3], 0x00u);
  EXPECT_EQ(bytes[4], 0x00u);
  EXPECT_EQ(bytes[5], 0x32u);  // separator
  EXPECT_EQ(bytes[6], 2u);     // section count
  // first section starts behind header and offset table
  EXPECT_EQ(bytes[7] | (bytes[8] << 8), 7 + 2 * 2);
}

TEST(IrProto, SerialiseRoundTrip)
{
  const int offset = 20;
  auto p = protoNec();
  IrProto q(p.serialise(offset), offset);

  EXPECT_EQ(q.getSectionCount(), 2);
  EXPECT_EQ(q.getClockPeriod(), 26315u);
  EXPECT_EQ(q.getBitCount(), 32);
  EXPECT_EQ(q.accessSection(0).getTiming(), 107000u);
  EXPECT_EQ(q.accessSection(1).getTiming(), TimingSection::PAUSE_IN_EOF);
  EXPECT_EQ(q.accessSection(1).getEoF().getCount(), 4);
  EXPECT_EQ(q.serialise(offset), p.serialise(offset));
}

TEST(IrProto, SerialiseRoundTripQuad)
{
  auto p = protoBelgacom();
  IrProto q(p.serialise(0), 0);

  ASSERT_EQ(q.getSectionCount(), 3);
  EXPECT_TRUE(q.accessSection(0).getData().isQuad());
  EXPECT_EQ(q.accessSection(0).getData().accessQuantum(3)[1], Item(false, 786));
  EXPECT_EQ(q.accessSection(2).getEoF().getCount(), 1);
  EXPECT_EQ(q.serialise(0), p.serialise(0));
}

TEST(IrProto, ParseTooSmall)
{
  IrProto p(std::vector<uint8_t>( { 0x01, 0x00, 0x00 }), 0);
  EXPECT_TRUE(p.isEmpty());
}

TEST(IrProto, ParseInvalidHeaderRejected)
{
  auto bytes = protoNec().serialise(0);

  auto badStart = bytes;
  badStart[0] = 0x02;
  EXPECT_TRUE(IrProto(badStart, 0).isEmpty());

  auto badSeparator = bytes;
  badSeparator[5] = 0x33;
  EXPECT_TRUE(IrProto(badSeparator, 0).isEmpty());
}

TEST(IrProto, ParseOffsetOutsideDataRejected)
{
  auto bytes = protoNec().serialise(0);
  bytes[7] = 0xFF;
  bytes[8] = 0xFF;
  EXPECT_TRUE(IrProto(bytes, 0).isEmpty());
}

TEST(IrProto, ParseTruncatedOffsetTableRejected)
{
  // section count 2, but offset table is cut off
  std::vector<uint8_t> bytes = { 0x01, 0xCB, 0x66, 0x00, 0x00, 0x32, 0x02, 0x0B };
  EXPECT_TRUE(IrProto(bytes, 0).isEmpty());
}

// ---------------------------------------------------------------------------
// compress
// ---------------------------------------------------------------------------

TEST(IrProtoCompress, Empty)
{
  EXPECT_TRUE(IrProto::compress( { }).empty());
}

TEST(IrProtoCompress, SingleItem)
{
  auto out = IrProto::compress( { { true, 100 } });
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(out[0], Item(true, 100));
}

TEST(IrProtoCompress, AlternatingUnchanged)
{
  const std::vector<Item> in = { { true, 100 }, { false, 200 }, { true, 300 }, {
      false, 400 } };
  EXPECT_EQ(IrProto::compress(in), in);
}

TEST(IrProtoCompress, MergesSameLevel)
{
  auto out = IrProto::compress( { { true, 100 }, { true, 200 }, { false, 300 } });
  const std::vector<Item> expected = { { true, 300 }, { false, 300 } };
  EXPECT_EQ(out, expected);
}

TEST(IrProtoCompress, MergesMultiple)
{
  auto out = IrProto::compress( { { false, 10000 }, { false, 10000 }, { false,
      10000 } });
  const std::vector<Item> expected = { { false, 30000 } };
  EXPECT_EQ(out, expected);
}

TEST(IrProtoCompress, SplitsOverflow)
{
  auto out = IrProto::compress( { { true, 30000 }, { true, 30000 } });
  const std::vector<Item> expected = { { true, 32767 }, { true, 27233 } };
  EXPECT_EQ(out, expected);
}

// ---------------------------------------------------------------------------
// timing stream
// ---------------------------------------------------------------------------

TEST(IrProtoStream, SingleFrame)
{
  IrProto p(26315, { sectionTiny(TimingSection::PAUSE_IN_EOF, 2) });
  TimingStream out;
  ASSERT_EQ(p.serialiseIrStream(out, { { 0, { 0, 1 } } }), Status::OK);

  // SoF, symbol 0, symbol 1, EoF
  ASSERT_EQ(out.timings().size(), 4u);
  expectPair(out, 0, 9000, 4500);
  expectPair(out, 1, 560, 560);
  expectPair(out, 2, 560, 1680);
  expectPair(out, 3, 560, 0);
}

TEST(IrProtoStream, FrameStartingWithPause)
{
  auto s = makeSection(0, TimingSection::PAUSE_IN_EOF, makeHeader( { { false,
      1000 } }), TimingSectionIrPayload(), makeHeader( { { true, 560 } }));
  IrProto p(26315, { s });
  TimingStream out;
  ASSERT_EQ(p.serialiseIrStream(out, { { 0, { } } }), Status::OK);
  ASSERT_EQ(out.timings().size(), 2u);
  expectPair(out, 0, 0, 1000);
  expectPair(out, 1, 560, 0);
}

TEST(IrProtoStream, UnknownSectionIsSkipped)
{
  IrProto p(26315, { sectionTiny(TimingSection::PAUSE_IN_EOF, 2) });
  TimingStream out;
  ASSERT_EQ(p.serialiseIrStream(out, { { 7, { 0, 1 } } }), Status::OK);
  EXPECT_TRUE(out.timings().empty());
}

TEST(IrProtoStream, NecDataAndRepeatFrame)
{
  // LG 82UN85006LA
  Code code(0, protoNec(), "0x0000F40101010020DF32CD010100");
  TimingStream out;
  ASSERT_EQ(protoNec().serialiseIrStream(out, code.getData()), Status::OK);

  // SoF + 32 symbols + EoF mark + rest of the 107ms interval + repeat frame (SoF, EoF, 2 pauses)
  ASSERT_EQ(out.timings().size(), 39u);
  expectPair(out, 0, 8990, 4490);
  expectPair(out, 1, 568, 552);    // first symbol: 0
  expectPair(out, 3, 568, 1662);   // third symbol: 1
  expectPair(out, 15, 568, 1662);
  expectPair(out, 32, 568, 1662);  // last symbol: 1
  expectPair(out, 33, 568, 32767); // EoF + interval
  expectPair(out, 34, 0, 6585);
  expectPair(out, 35, 8990, 2230); // repeat frame
  expectPair(out, 36, 568, 32767);
  expectPair(out, 37, 0, 32767);
  expectPair(out, 38, 0, 30543);
}

TEST(IrProtoStream, NecIntervalIsComplete)
{
  Code code(0, protoNec(), "0x0000F40101010020DF32CD010100");
  std::vector<Item> items;
  protoNec().accessSection(0).serialiseIrStream(items, code.getData()[0].second);

  uint32_t total = 0;
  for (const auto &item : items) {
    total += item.second;
  }
  EXPECT_EQ(total, 107000u);
}

TEST(IrProtoStream, NecRepeatsSendRepeatFrameOnlyInData)
{
  // 3 repeat frames after the data frame
  Code code(0, protoNec(), "0x0000F40103010020DF32CD010100");
  TimingStream out;
  ASSERT_EQ(protoNec().serialiseIrStream(out, code.getData()), Status::OK);
  // 39 timings for the first repeat frame, 4 more for each additional one
  EXPECT_EQ(out.timings().size(), 39u + 2u * 4u);
}

TEST(IrProtoStream, BelgacomThreeFrames)
{
  // Belgacom ISB6030: repeat frame once, finish with 2 frames. all 3 frames have different EoF
  Code code(3, protoBelgacom(), "0x0300F4010100010022C026F7020122C026F70222C0A6F7");
  TimingStream out;
  ASSERT_EQ(protoBelgacom().serialiseIrStream(out, code.getData()), Status::OK);

  ASSERT_EQ(out.timings().size(), 57u);
  // frame 0
  expectPair(out, 0, 418, 286);
  expectPair(out, 1, 158, 286);    // symbol 0 = 0
  expectPair(out, 2, 158, 620);    // symbol 1 = 2
  expectPair(out, 9, 158, 286);    // symbol 8 = 0
  expectPair(out, 16, 158, 786);   // symbol 15 = 3
  expectPair(out, 17, 160, 32767); // EoF
  expectPair(out, 18, 0, 32767);
  expectPair(out, 19, 0, 24766);
  // frame 1
  expectPair(out, 20, 418, 286);
  expectPair(out, 36, 158, 786);
  expectPair(out, 37, 158, 32767); // EoF with merged pauses
  expectPair(out, 38, 0, 7364);
  // frame 2: ninth symbol is 2, no EoF pause
  expectPair(out, 39, 418, 286);
  expectPair(out, 48, 158, 620);
  expectPair(out, 55, 158, 786);
  expectPair(out, 56, 158, 0);
}

// ===========================================================================
// File – unit tests (no real file)
// ===========================================================================

TEST(IrProtoFile, DefaultEmpty)
{
  File f;
  EXPECT_TRUE(f.isEmpty());
  EXPECT_EQ(f.getProtocolCount(), 0);
}

TEST(IrProtoFile, NonExistentFile)
{
  File f;
  EXPECT_EQ(f.parse("/tmp/does_not_exist_irproto.bin"), Status::ERROR_FILE);
}

TEST(IrProtoFile, ParseFailureClearsProtocols)
{
  File f;
  f.appendProtocol(protoNec());
  EXPECT_EQ(f.parse("/tmp/does_not_exist_irproto.bin"), Status::ERROR_FILE);
  EXPECT_TRUE(f.isEmpty());
}

TEST(IrProtoFile, OutOfRangeAccessThrows)
{
  File f;
  EXPECT_THROW(f.accessProtocol(0), std::out_of_range);
}

TEST(IrProtoFile, AppendAndAccess)
{
  File f;
  IrProto proto;
  auto idx = f.appendProtocol(proto);
  EXPECT_EQ(idx, 0u);
  EXPECT_EQ(f.getProtocolCount(), 1);
  // accessProtocol should not throw
  EXPECT_NO_THROW(f.accessProtocol(0));
}

TEST(IrProtoFile, AppendReturnsIndex)
{
  File f;
  EXPECT_EQ(f.appendProtocol(protoNec()), 0u);
  EXPECT_EQ(f.appendProtocol(protoBelgacom()), 1u);
  EXPECT_EQ(f.accessProtocol(1).getSectionCount(), 3);
}

TEST(IrProtoFile, InsertProtocol)
{
  File f;
  f.appendProtocol(protoNec());
  EXPECT_TRUE(f.insertProtocol(protoBelgacom(), 0));
  EXPECT_TRUE(f.insertProtocol(protoNec(), 2));
  ASSERT_EQ(f.getProtocolCount(), 3);
  EXPECT_EQ(f.accessProtocol(0).getSectionCount(), 3);
  EXPECT_EQ(f.accessProtocol(1).getSectionCount(), 2);
  EXPECT_EQ(f.accessProtocol(2).getSectionCount(), 2);
}

TEST(IrProtoFile, InsertOutOfRangeRejected)
{
  File f;
  EXPECT_FALSE(f.insertProtocol(protoNec(), 1));
  EXPECT_FALSE(f.insertProtocol(protoNec(), -1));
  EXPECT_TRUE(f.isEmpty());
}

TEST(IrProtoFile, RemoveProtocol)
{
  File f;
  IrProto p;
  f.appendProtocol(p);
  f.appendProtocol(p);
  f.removeProtocol(0);
  EXPECT_EQ(f.getProtocolCount(), 1);
}

TEST(IrProtoFile, RemoveKeepsOrder)
{
  File f;
  f.appendProtocol(protoNec());
  f.appendProtocol(protoBelgacom());
  f.removeProtocol(0);
  ASSERT_EQ(f.getProtocolCount(), 1);
  EXPECT_EQ(f.accessProtocol(0).getSectionCount(), 3);
}

TEST(IrProtoFile, RemoveOutOfRangeNoThrow)
{
  File f;
  EXPECT_NO_THROW(f.removeProtocol(99));
}

TEST(IrProtoFile, WriteToInvalidPath)
{
  File f;
  EXPECT_EQ(f.serialise("/nonexistent_dir/irproto.bin"), Status::ERROR_FILE);
}

TEST(IrProtoFile, SerialiseEmptyContainsHeader)
{
  File f;
  auto raw = f.serialise();
  // wrapper(8) + inner header(6) >= 14 bytes
  ASSERT_GE(raw.size(), 14u);
  // inner header starts at byte 8
  EXPECT_EQ(raw[8], 0x01u);
  EXPECT_EQ(raw[9], 0x01u);
  EXPECT_EQ(raw[10], 0x05u);
  EXPECT_EQ(raw[11], 0x00u);
  EXPECT_EQ(raw[12], 0x00u);
  EXPECT_EQ(raw[13], 0x01u);
}

TEST(IrProtoFile, SerialiseReturnsCrc)
{
  File f;
  uint32_t crc = 0;
  f.serialise(&crc);
  EXPECT_NE(crc, 0u);
}

TEST(IrProtoFile, SerialiseWrapper)
{
  File f;
  f.appendProtocol(protoNec());
  uint32_t crc = 0;
  auto raw = f.serialise(&crc);

  // size of data, empty field, crc of data
  std::vector<uint8_t> payload(raw.begin() + 8, raw.end());
  EXPECT_EQ(raw[4] | (raw[5] << 8), static_cast<int>(payload.size()));
  EXPECT_EQ(raw[6], 0u);
  EXPECT_EQ(raw[7], 0u);
  EXPECT_EQ(crc, lib::calcCrc32(payload));
}

TEST(IrProtoFile, ParseEmptyFile)
{
  File f;
  f.appendProtocol(protoNec());
  EXPECT_EQ(parseBytes(File().serialise(), f), Status::OK);
  EXPECT_TRUE(f.isEmpty());
}

TEST(IrProtoFile, WriteAndParseRoundTrip)
{
  File f;
  f.appendProtocol(protoNec());
  f.appendProtocol(protoBelgacom());
  auto expected = f.serialise();

  auto path = tempPath("irproto_test_roundtrip.bin");
  ASSERT_EQ(f.serialise(path), Status::OK);

  File g;
  ASSERT_EQ(g.parse(path), Status::OK);
  ASSERT_EQ(g.getProtocolCount(), 2);
  EXPECT_EQ(g.accessProtocol(0).getSectionCount(), 2);
  EXPECT_EQ(g.accessProtocol(1).getSectionCount(), 3);
  EXPECT_EQ(g.accessProtocol(0).accessSection(0).getTiming(), 107000u);
  EXPECT_EQ(g.accessProtocol(1).accessSection(2).getEoF().getCount(), 1);
  EXPECT_EQ(g.serialise(), expected);
  EXPECT_EQ(readFile(path), expected);

  std::filesystem::remove(path);
}

TEST(IrProtoFile, ConstructFromFilename)
{
  File f;
  f.appendProtocol(protoBelgacom());
  auto path = tempPath("irproto_test_ctor.bin");
  ASSERT_EQ(f.serialise(path), Status::OK);

  File g(path);
  EXPECT_EQ(g.getProtocolCount(), 1);

  std::filesystem::remove(path);
}

TEST(IrProtoFile, ParseTooSmall)
{
  File f;
  EXPECT_EQ(parseBytes( { 0x00, 0x01, 0x02, 0x03, 0x04 }, f), Status::ERROR_SIZE);
}

TEST(IrProtoFile, ParseOuterSizeMismatch)
{
  auto raw = File().serialise();
  raw[4] = raw[4] + 1;
  File f;
  EXPECT_EQ(parseBytes(raw, f), Status::ERROR_OUTER_SIZE);
}

TEST(IrProtoFile, ParseOuterFormatInvalid)
{
  auto raw = File().serialise();
  raw[6] = 1;
  File f;
  EXPECT_EQ(parseBytes(raw, f), Status::ERROR_OUTER_FILE_FORMAT);
}

TEST(IrProtoFile, ParseCrcMismatch)
{
  auto raw = File().serialise();
  raw[0] = raw[0] ^ 0xFF;
  File f;
  EXPECT_EQ(parseBytes(raw, f), Status::ERROR_OUTER_CRC);
}

TEST(IrProtoFile, ParseHeaderMismatch)
{
  auto raw = File().serialise();
  raw[8] = 0x02;
  File f;
  EXPECT_EQ(parseBytes(fixWrapper(raw), f), Status::ERROR_FILE_FORMAT);
}

TEST(IrProtoFile, ParseOffsetOutsideFile)
{
  File source;
  source.appendProtocol(protoNec());
  auto raw = source.serialise();
  // offset table of protocol 0
  raw[16] = 0xFF;
  raw[17] = 0xFF;
  File f;
  EXPECT_EQ(parseBytes(fixWrapper(raw), f), Status::ERROR_SIZE);
}

TEST(IrProtoFile, ParseProtocolCountLargerThanFile)
{
  auto raw = File().serialise();
  raw[14] = 0xFF;
  raw[15] = 0xFF;
  File f;
  EXPECT_EQ(parseBytes(fixWrapper(raw), f), Status::ERROR_SIZE);
}

TEST(IrProtoFile, ParseOffsetsNotAscending)
{
  File source;
  source.appendProtocol(protoNec());
  source.appendProtocol(protoBelgacom());
  auto raw = source.serialise();
  // swap offset table entries of protocol 0 and 1
  std::swap(raw[16], raw[18]);
  std::swap(raw[17], raw[19]);
  File f;
  EXPECT_NE(parseBytes(fixWrapper(raw), f), Status::OK);
}

TEST(IrProtoFile, ParseInvalidProtocolBlockRejected)
{
  File source;
  source.appendProtocol(protoNec());
  auto raw = source.serialise();
  // first protocol starts behind wrapper(8), header(6), count(2) and offset table(2)
  raw[18] = 0x7F;
  File f;
  EXPECT_NE(parseBytes(fixWrapper(raw), f), Status::OK);
}

TEST(IrProtoFile, SerialiseIrStreamInvalidIndex)
{
  File f;
  f.appendProtocol(protoNec());
  TimingStream out;
  EXPECT_EQ(f.serialiseIrStream(out, 1, { { 0, { } } }), Status::ERROR_INDEX);
  EXPECT_TRUE(out.timings().empty());
}

TEST(IrProtoFile, SerialiseIrStreamSelectsProtocol)
{
  File f;
  f.appendProtocol(protoNec());
  f.appendProtocol(protoBelgacom());

  Code code(1, f.accessProtocol(1), "0x0100F4010100010022C026F7020122C026F70222C0A6F7");
  TimingStream out;
  auto res = f.serialiseIrStream(out, code.getIndex(), code.getData());
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(out.timings().size(), 57u);
}

// ===========================================================================
// Real IrProto.bin fixture
// ===========================================================================

class RealIrProtoTest: public ::testing::Test
{
  protected:
    File file;
    const std::string binPath = REAL_IRPROTO_BIN_PATH;

    void SetUp() override
    {
      auto status = file.parse(binPath);
      ASSERT_EQ(status, Status::OK)<< "Could not parse " << binPath;
    }
  };

// ---------------------------------------------------------------------------
// File-level structure
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, NotEmpty)
{
  EXPECT_FALSE(file.isEmpty());
}

TEST_F(RealIrProtoTest, ProtocolCount)
{
  EXPECT_EQ(file.getProtocolCount(), 8);
}

TEST_F(RealIrProtoTest, OutOfRangeAccessThrows)
{
  EXPECT_THROW(file.accessProtocol(8), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Protocol 0 (NEC-like, 2 sections: data + repeat)
// clockPeriod=26315ns (~38kHz), section[0] 32-bit data, section[1] repeat
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, Proto0SectionCount)
{
  EXPECT_EQ(file.accessProtocol(0).getSectionCount(), 2);
}

TEST_F(RealIrProtoTest, Proto0Clock)
{
  EXPECT_NEAR(file.accessProtocol(0).getClock(), 38000.0, 200.0);
}

TEST_F(RealIrProtoTest, Proto0Section0IsDataFrame)
{
  const auto &s = file.accessProtocol(0).accessSection(0);
  EXPECT_EQ(s.getBitCount(), 32u);
  EXPECT_TRUE(s.getData().isBoolean());
  EXPECT_EQ(s.getData().getTperQantum(), 2u);
  EXPECT_FALSE(s.hasToggle());
  EXPECT_NE(s.getTiming(), TimingSection::PAUSE_IN_EOF);
}

TEST_F(RealIrProtoTest, Proto0Section0SoF)
{
  const auto &sof = file.accessProtocol(0).accessSection(0).getSoF();
  ASSERT_EQ(sof.getCount(), 2);
  EXPECT_EQ(sof.accessStream()[0].first, true);   // mark
  EXPECT_EQ(sof.accessStream()[0].second, 8990u);
  EXPECT_EQ(sof.accessStream()[1].first, false);  // pause
  EXPECT_EQ(sof.accessStream()[1].second, 4490u);
}

TEST_F(RealIrProtoTest, Proto0Section0EoF)
{
  const auto &eof = file.accessProtocol(0).accessSection(0).getEoF();
  ASSERT_EQ(eof.getCount(), 1);
  EXPECT_EQ(eof.accessStream()[0].first, true);
  EXPECT_EQ(eof.accessStream()[0].second, 568u);
}

TEST_F(RealIrProtoTest, Proto0Section0Payload)
{
  const auto &d = file.accessProtocol(0).accessSection(0).getData();
  ASSERT_EQ(d.accessQuantum(0).size(), 2u);
  ASSERT_EQ(d.accessQuantum(1).size(), 2u);
  // false: mark=568, pause=552
  EXPECT_EQ(d.accessQuantum(0)[0].second, 568u);
  EXPECT_EQ(d.accessQuantum(0)[1].second, 552u);
  // true:  mark=568, pause=1662
  EXPECT_EQ(d.accessQuantum(1)[0].second, 568u);
  EXPECT_EQ(d.accessQuantum(1)[1].second, 1662u);
}

TEST_F(RealIrProtoTest, Proto0Section1IsRepeatFrame)
{
  const auto &s = file.accessProtocol(0).accessSection(1);
  EXPECT_EQ(s.getBitCount(), 0u);
  EXPECT_TRUE(s.getData().isEmpty());
  EXPECT_EQ(s.getData().getQuantisation(), 0u);
  EXPECT_EQ(s.getTiming(), TimingSection::PAUSE_IN_EOF);
}

TEST_F(RealIrProtoTest, Proto0Section1SoF)
{
  const auto &sof = file.accessProtocol(0).accessSection(1).getSoF();
  ASSERT_EQ(sof.getCount(), 2);
  EXPECT_EQ(sof.accessStream()[0].second, 8990u);
  EXPECT_EQ(sof.accessStream()[1].second, 2230u);
}

TEST_F(RealIrProtoTest, Proto0Section1EoF)
{
  // EoF has 4 items including silence: 568, 32767, 32767, 30543
  const auto &eof = file.accessProtocol(0).accessSection(1).getEoF();
  ASSERT_EQ(eof.getCount(), 4);
  EXPECT_EQ(eof.accessStream()[0].second, 568u);
  EXPECT_EQ(eof.accessStream()[1].second, 32767u);
  EXPECT_EQ(eof.accessStream()[3].second, 30543u);
}

// ---------------------------------------------------------------------------
// Protocol 1 (NEC-like, 1 section, different timings)
// clockPeriod=26315ns, SoF=(4500,4500), EoF has 3 items
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, Proto1SectionCount)
{
  EXPECT_EQ(file.accessProtocol(1).getSectionCount(), 1);
}

TEST_F(RealIrProtoTest, Proto1Section0SoF)
{
  const auto &sof = file.accessProtocol(1).accessSection(0).getSoF();
  ASSERT_EQ(sof.getCount(), 2);
  EXPECT_EQ(sof.accessStream()[0].second, 4500u);
  EXPECT_EQ(sof.accessStream()[1].second, 4500u);
}

TEST_F(RealIrProtoTest, Proto1Section0Payload)
{
  const auto &d = file.accessProtocol(1).accessSection(0).getData();
  // false: mark=600, pause=1650  true: mark=600, pause=500
  EXPECT_EQ(d.accessQuantum(0)[0].second, 600u);
  EXPECT_EQ(d.accessQuantum(0)[1].second, 1650u);
  EXPECT_EQ(d.accessQuantum(1)[0].second, 600u);
  EXPECT_EQ(d.accessQuantum(1)[1].second, 500u);
}

// ---------------------------------------------------------------------------
// Protocol 2 (Philips RC-6 / multi-section, 3 sections, 36kHz)
// clockPeriod=27777ns, 3 sections
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, Proto2SectionCount)
{
  EXPECT_EQ(file.accessProtocol(2).getSectionCount(), 3);
}

TEST_F(RealIrProtoTest, Proto2Clock)
{
  EXPECT_NEAR(file.accessProtocol(2).getClock(), 36000.0, 200.0);
}

TEST_F(RealIrProtoTest, Proto2Section0BitCount)
{
  EXPECT_EQ(file.accessProtocol(2).accessSection(0).getBitCount(), 4u);
}

TEST_F(RealIrProtoTest, Proto2Section1HasToggle)
{
  const auto &s = file.accessProtocol(2).accessSection(1);
  EXPECT_EQ(s.getBitCount(), 1u);
  EXPECT_TRUE(s.hasToggle());
  EXPECT_EQ(s.getToggle(), 0u);
}

TEST_F(RealIrProtoTest, Proto2Section2BitCount)
{
  EXPECT_EQ(file.accessProtocol(2).accessSection(2).getBitCount(), 16u);
}

TEST_F(RealIrProtoTest, Proto2Section2EoF)
{
  // EoF is 3 silence items: 32767, 32767, 18866
  const auto &eof = file.accessProtocol(2).accessSection(2).getEoF();
  ASSERT_EQ(eof.getCount(), 3);
  EXPECT_EQ(eof.accessStream()[0].first, false);
  EXPECT_EQ(eof.accessStream()[0].second, 32767u);
  EXPECT_EQ(eof.accessStream()[2].second, 18866u);
}

// ---------------------------------------------------------------------------
// Protocol 4 (Vu+ / METZ-like, 36.2kHz, SoF has 13 items)
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, Proto4SectionCount)
{
  EXPECT_EQ(file.accessProtocol(4).getSectionCount(), 1);
}

TEST_F(RealIrProtoTest, Proto4Clock)
{
  EXPECT_NEAR(file.accessProtocol(4).getClock(), 36200.0, 200.0);
}

TEST_F(RealIrProtoTest, Proto4Section0BitCount)
{
  EXPECT_EQ(file.accessProtocol(4).accessSection(0).getBitCount(), 30u);
}

TEST_F(RealIrProtoTest, Proto4Section0SoFCount)
{
  // Long preamble with 13 timing items
  EXPECT_EQ(file.accessProtocol(4).accessSection(0).getSoF().getCount(), 13);
}

TEST_F(RealIrProtoTest, Proto4Section0SoFFirst)
{
  const auto &sof = file.accessProtocol(4).accessSection(0).getSoF();
  EXPECT_EQ(sof.accessStream()[0].first, true);
  EXPECT_EQ(sof.accessStream()[0].second, 2632u);
  EXPECT_EQ(sof.accessStream()[1].second, 900u);
}

TEST_F(RealIrProtoTest, Proto4Section0HasToggle)
{
  const auto &s = file.accessProtocol(4).accessSection(0);
  EXPECT_TRUE(s.hasToggle());
  EXPECT_EQ(s.getToggle(), 14u);
}

// ---------------------------------------------------------------------------
// Protocol 5 (XBox / Sony-like, 56kHz)
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, Proto5Clock)
{
  EXPECT_NEAR(file.accessProtocol(5).getClock(), 56000.0, 200.0);
}

TEST_F(RealIrProtoTest, Proto5Section0BitCount)
{
  EXPECT_EQ(file.accessProtocol(5).accessSection(0).getBitCount(), 24u);
}

TEST_F(RealIrProtoTest, Proto5Section0SoF)
{
  const auto &sof = file.accessProtocol(5).accessSection(0).getSoF();
  ASSERT_EQ(sof.getCount(), 2);
  EXPECT_EQ(sof.accessStream()[0].second, 3929u);
  EXPECT_EQ(sof.accessStream()[1].second, 4000u);
}

TEST_F(RealIrProtoTest, Proto5Section0Payload)
{
  const auto &d = file.accessProtocol(5).accessSection(0).getData();
  EXPECT_EQ(d.accessQuantum(0)[1].second, 1000u);
  EXPECT_EQ(d.accessQuantum(1)[1].second, 2000u);
}

// ---------------------------------------------------------------------------
// Protocol 6 (Led RGB / biphase, 36kHz, SoF 1 item)
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, Proto6Section0BitCount)
{
  EXPECT_EQ(file.accessProtocol(6).accessSection(0).getBitCount(), 13u);
}

TEST_F(RealIrProtoTest, Proto6Section0SoFSingleItem)
{
  const auto &sof = file.accessProtocol(6).accessSection(0).getSoF();
  ASSERT_EQ(sof.getCount(), 1);
  EXPECT_EQ(sof.accessStream()[0].second, 889u);
}

// ---------------------------------------------------------------------------
// Write: serialise + binary compare with original
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, SerialisedBinaryMatchesOriginal)
{
  // Read original file bytes
  std::ifstream orig(binPath, std::ios::binary);
  ASSERT_TRUE(orig.is_open());
  std::vector<uint8_t> expected( { std::istreambuf_iterator<char>(orig),
      std::istreambuf_iterator<char>() });

  // Serialise and compare
  auto actual = file.serialise();

  ASSERT_EQ(actual.size(), expected.size())<< "Serialised size " << actual.size()
  << " != original size " << expected.size();

  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(actual[i], expected[i]) << "Byte mismatch at offset " << i
        << " (got 0x" << std::hex << static_cast<int>(actual[i])
        << ", expected 0x" << static_cast<int>(expected[i]) << std::dec << ")";
    if (actual[i] != expected[i]) {
      break; // stop at first diff to keep output readable
    }
  }
}

TEST_F(RealIrProtoTest, WriteFileBinaryMatchesOriginal)
{
  auto outPath =
      (std::filesystem::temp_directory_path() / "SsIr_roundtrip.bin").string();

  auto status = file.serialise(outPath);
  ASSERT_EQ(status, Status::OK);

  // Read original
  std::ifstream orig(binPath, std::ios::binary);
  ASSERT_TRUE(orig.is_open());
  std::vector<uint8_t> expected( { std::istreambuf_iterator<char>(orig),
      std::istreambuf_iterator<char>() });

  // Read written file
  std::ifstream written(outPath, std::ios::binary);
  ASSERT_TRUE(written.is_open());
  std::vector<uint8_t> actual( { std::istreambuf_iterator<char>(written),
      std::istreambuf_iterator<char>() });

  ASSERT_EQ(actual.size(), expected.size())<< "Written file size " << actual.size()
  << " != original size " << expected.size();

  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(actual[i], expected[i]) << "Byte mismatch at offset " << i
        << " (got 0x" << std::hex << static_cast<int>(actual[i])
        << ", expected 0x" << static_cast<int>(expected[i]) << std::dec << ")";
    if (actual[i] != expected[i]) {
      break;
    }
  }

  std::filesystem::remove(outPath);
}

TEST_F(RealIrProtoTest, SerialiseReturnsCrc)
{
  uint32_t crc = 0;
  file.serialise(&crc);
  // CRC32 of data section must match what was in the file wrapper
  EXPECT_EQ(crc, 0x74693195u);
}

TEST_F(RealIrProtoTest, SerialiseEachProtocolRoundTrips)
{
  for (int i = 0; i < file.getProtocolCount(); i++) {
    auto bytes = file.accessProtocol(i).serialise(0);
    IrProto copy(bytes, 0);
    EXPECT_EQ(copy.getSectionCount(), file.accessProtocol(i).getSectionCount())
        << "protocol " << i;
    EXPECT_EQ(copy.serialise(0), bytes) << "protocol " << i;
  }
}

// ---------------------------------------------------------------------------
// Device command examples – end-to-end: Code::parse + File::serialiseIrStream
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, LG_FlatCommandProducesTimingStream)
{
  // LG 82UN85006LA: proto=0, data frame + repeat frame, 32 bits
  Code code(0, file.accessProtocol(0), "0x0000F40101010020DF32CD010100");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  // 32 data bits -> 32 mark/pause pairs + SoF(2) + EoF(1) + repeat section SoF/EoF
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 39u);
  EXPECT_EQ(out.timings()[0].mark_us, 8990u);
  EXPECT_EQ(out.timings()[0].pause_us, 4490u);
  EXPECT_EQ(out.timings()[15].mark_us, 568u);
  EXPECT_EQ(out.timings()[15].pause_us, 1662u);
}

TEST_F(RealIrProtoTest, Samsung_SingleSectionCommandProducesTimingStream)
{
  // Samsung SV-6332X: proto=1, repeat group, repeats=3
  Code code(1, file.accessProtocol(1), "0x0100F401030001005F5F778800");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 105u);
  EXPECT_EQ(out.timings()[0].mark_us, 4500u);
  EXPECT_EQ(out.timings()[0].pause_us, 4500u);
  EXPECT_EQ(out.timings()[72].mark_us, 600u);
  EXPECT_EQ(out.timings()[72].pause_us, 500u);
}

TEST_F(RealIrProtoTest, Philips_MultiSectionCommandProducesTimingStream)
{
  // Philips BDP-2700: proto=2, repeat group with 3 sections
  Code code(2, file.accessProtocol(2), "0x0200F4010300030070010002B9FF00");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 63u);
  EXPECT_EQ(out.timings()[0].mark_us, 2662u);
  EXPECT_EQ(out.timings()[0].pause_us, 870u);
  EXPECT_EQ(out.timings()[17].mark_us, 457u);
  EXPECT_EQ(out.timings()[18].pause_us, 32767u);
}

TEST_F(RealIrProtoTest, VuPlus_SingleSectionCommandProducesTimingStream)
{
  // Vu+ DUO 2: proto=4, repeat group, repeats=1
  Code code(4, file.accessProtocol(4), "0x0400C80001000100FEB5BFFC00");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 34u);
  EXPECT_EQ(out.timings()[0].mark_us, 2632u);
  EXPECT_EQ(out.timings()[0].pause_us, 900u);
  EXPECT_EQ(out.timings()[17].mark_us, 882u);
  EXPECT_EQ(out.timings()[18].pause_us, 446u);
}

TEST_F(RealIrProtoTest, XBox_SingleSectionCommandProducesTimingStream)
{
  // Microsoft XBox: proto=5, 24 bits, 56kHz
  Code code(5, file.accessProtocol(5), "0x0500F40103000100530ACF00");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 78u);
  EXPECT_EQ(out.timings()[0].mark_us, 3929u);
  EXPECT_EQ(out.timings()[0].pause_us, 4000u);
  EXPECT_EQ(out.timings()[22].mark_us, 500u);
  EXPECT_EQ(out.timings()[22].pause_us, 2000u);
}

TEST_F(RealIrProtoTest, Led_SingleSectionCommandProducesTimingStream)
{
  // Led RGB-LED-1: proto=6, 13 bits
  Code code(6, file.accessProtocol(6), "0x0600F401030001009C0000");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 42u);
  EXPECT_EQ(out.timings()[0].mark_us, 889u);
  EXPECT_EQ(out.timings()[0].pause_us, 889u);
  EXPECT_EQ(out.timings()[25].mark_us, 889u);
  EXPECT_EQ(out.timings()[4].pause_us, 889u);
}

TEST_F(RealIrProtoTest, Onkyo_FlatCommandMatchesLG)
{
  // same protocol as the LG code, only the symbols differ
  Code lg(0, file.accessProtocol(0), "0x0000F40101010020DF32CD010100");
  Code onkyo(0, file.accessProtocol(0), "0x0000C8000101004B407B84010100");
  TimingStream lgOut;
  TimingStream onkyoOut;
  ASSERT_EQ(file.serialiseIrStream(lgOut, 0, lg.getData()), Status::OK);
  ASSERT_EQ(file.serialiseIrStream(onkyoOut, 0, onkyo.getData()), Status::OK);
  EXPECT_EQ(onkyoOut.timings().size(), lgOut.timings().size());
  EXPECT_EQ(onkyoOut.timings()[0].mark_us, 8990u);
  EXPECT_EQ(onkyoOut.timings()[0].pause_us, 4490u);
}

TEST_F(RealIrProtoTest, InvalidProtocolIndexReturnsError)
{
  Code code(0, file.accessProtocol(0), "0x0000F40101010020DF32CD010100");
  TimingStream out;
  // protocol index 99 doesn't exist
  auto status = file.serialiseIrStream(out, 99, code.getData());
  EXPECT_EQ(status, Status::ERROR_INDEX);
}
