// SPDX-License-Identifier: LGPL-2.1-or-later

/*
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

#include "file.h"
#include "code.h"

using namespace binary;
using namespace irProto;

#ifndef REAL_IRPROTO_BIN_PATH
#define REAL_IRPROTO_BIN_PATH "IrProto.bin"
#endif

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

// ===========================================================================
// TimingSectionIrPayload
// ===========================================================================

TEST(TimingSectionIrPayload, DefaultEmpty)
{
  TimingSectionIrPayload p;
  EXPECT_TRUE(p.isEmpty());
  EXPECT_FALSE(p.haveFalse());
  EXPECT_FALSE(p.haveTrue());
}

TEST(TimingSectionIrPayload, SetFalsePairRejectsBadSize)
{
  TimingSectionIrPayload p;
  p.setFalsePair( { { true, 500 } });  // size 1, must be 2
  EXPECT_FALSE(p.haveFalse());
}

TEST(TimingSectionIrPayload, SetPairsAndAccess)
{
  TimingSectionIrPayload p;
  p.setFalsePair( { { true, 568 }, { false, 552 } });
  p.setTruePair( { { true, 568 }, { false, 1662 } });

  ASSERT_EQ(p.accessFalsePair().size(), 2u);
  ASSERT_EQ(p.accessTruePair().size(), 2u);

  EXPECT_EQ(p.accessFalsePair()[0], Item(true, 568));
  EXPECT_EQ(p.accessFalsePair()[1], Item(false, 552));
  EXPECT_EQ(p.accessTruePair()[0], Item(true, 568));
  EXPECT_EQ(p.accessTruePair()[1], Item(false, 1662));
}

TEST(TimingSectionIrPayload, SerialiseIrStreamFalse)
{
  TimingSectionIrPayload p;
  p.setFalsePair( { { true, 568 }, { false, 552 } });
  p.setTruePair( { { true, 568 }, { false, 1662 } });

  std::vector<Item> out;
  p.serialiseIrStream(false, out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0], Item(true, 568));
  EXPECT_EQ(out[1], Item(false, 552));
}

TEST(TimingSectionIrPayload, SerialiseIrStreamTrue)
{
  TimingSectionIrPayload p;
  p.setFalsePair( { { true, 568 }, { false, 552 } });
  p.setTruePair( { { true, 568 }, { false, 1662 } });

  std::vector<Item> out;
  p.serialiseIrStream(true, out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[1], Item(false, 1662));
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

TEST(IrProtoFile, RemoveProtocol)
{
  File f;
  IrProto p;
  f.appendProtocol(p);
  f.appendProtocol(p);
  f.removeProtocol(0);
  EXPECT_EQ(f.getProtocolCount(), 1);
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
  EXPECT_EQ(s.getCtrl0(), TimingSection::Ctrl0::IS_DATA_FRAME);
  EXPECT_EQ(s.getCtrl1(), TimingSection::Ctrl1::DATA_TWO_PAIRS);
  EXPECT_EQ(s.getBitCount(), 32u);
  EXPECT_FALSE(s.hasToggle());
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
  ASSERT_EQ(d.accessFalsePair().size(), 2u);
  ASSERT_EQ(d.accessTruePair().size(), 2u);
  // false: mark=568, pause=552
  EXPECT_EQ(d.accessFalsePair()[0].second, 568u);
  EXPECT_EQ(d.accessFalsePair()[1].second, 552u);
  // true:  mark=568, pause=1662
  EXPECT_EQ(d.accessTruePair()[0].second, 568u);
  EXPECT_EQ(d.accessTruePair()[1].second, 1662u);
}

TEST_F(RealIrProtoTest, Proto0Section1IsRepeatFrame)
{
  const auto &s = file.accessProtocol(0).accessSection(1);
  EXPECT_EQ(s.getCtrl0(), TimingSection::Ctrl0::IS_REPEAT_FRAME);
  EXPECT_EQ(s.getCtrl1(), TimingSection::Ctrl1::NO_PAYLOAD);
  EXPECT_EQ(s.getBitCount(), 0u);
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
  EXPECT_EQ(d.accessFalsePair()[0].second, 600u);
  EXPECT_EQ(d.accessFalsePair()[1].second, 1650u);
  EXPECT_EQ(d.accessTruePair()[0].second, 600u);
  EXPECT_EQ(d.accessTruePair()[1].second, 500u);
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
  EXPECT_EQ(d.accessFalsePair()[1].second, 1000u);
  EXPECT_EQ(d.accessTruePair()[1].second, 2000u);
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

// ---------------------------------------------------------------------------
// Device command examples – end-to-end: Code::parse + File::serialiseIrStream
// ---------------------------------------------------------------------------

TEST_F(RealIrProtoTest, LG_FlatCommandProducesTimingStream)
{
  // LG 82UN85006LA: proto=0, FLAT, 32 bits
  Code code("0x0000F40101010020DF32CD010100");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  // 32 data bits -> 32 mark/pause pairs + SoF(2) + EoF(1) + repeat section SoF/EoF
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 39);
  EXPECT_EQ(out.timings()[0].mark_us, 8990u);
  EXPECT_EQ(out.timings()[0].pause_us, 4490u);
  EXPECT_EQ(out.timings()[15].mark_us, 568u);
  EXPECT_EQ(out.timings()[15].pause_us, 1662u);
}

TEST_F(RealIrProtoTest, Samsung_SingleSectionCommandProducesTimingStream)
{
  // Samsung SV-6332X: proto=1, SINGLE_SECTION, tx=3
  Code code("0x0100F401030001005F5F778800");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 105);
  EXPECT_EQ(out.timings()[0].mark_us, 4500u);
  EXPECT_EQ(out.timings()[0].pause_us, 4500u);
  EXPECT_EQ(out.timings()[72].mark_us, 600u);
  EXPECT_EQ(out.timings()[72].pause_us, 500u);
}

TEST_F(RealIrProtoTest, Philips_MultiSectionCommandProducesTimingStream)
{
  // Philips BDP-2700: proto=2, MULTI_SECTION(3)
  Code code("0x0200F4010300030070010002B9FF00");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 63);
  EXPECT_EQ(out.timings()[0].mark_us, 2662u);
  EXPECT_EQ(out.timings()[0].pause_us, 870u);
  EXPECT_EQ(out.timings()[17].mark_us, 457u);
  EXPECT_EQ(out.timings()[18].pause_us, 32767u);
}

TEST_F(RealIrProtoTest, VuPlus_SingleSectionCommandProducesTimingStream)
{
  // Vu+ DUO 2: proto=4, SINGLE_SECTION, tx=1
  Code code("0x0400C80001000100FEB5BFFC00");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 34);
  EXPECT_EQ(out.timings()[0].mark_us, 2632u);
  EXPECT_EQ(out.timings()[0].pause_us, 900u);
  EXPECT_EQ(out.timings()[17].mark_us, 882u);
  EXPECT_EQ(out.timings()[18].pause_us, 446u);
}

TEST_F(RealIrProtoTest, XBox_SingleSectionCommandProducesTimingStream)
{
  // Microsoft XBox: proto=5, 24 bits, 56kHz
  Code code("0x0500F40103000100530ACF00");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 78);
  EXPECT_EQ(out.timings()[0].mark_us, 3929u);
  EXPECT_EQ(out.timings()[0].pause_us, 4000u);
  EXPECT_EQ(out.timings()[22].mark_us, 500u);
  EXPECT_EQ(out.timings()[22].pause_us, 2000u);
}

TEST_F(RealIrProtoTest, Led_SingleSectionCommandProducesTimingStream)
{
  // Led RGB-LED-1: proto=6, 16 bits
  Code code("0x0600F401030001009C0000");
  TimingStream out;
  auto status = file.serialiseIrStream(out, code.getIndex(), code.getData());
  EXPECT_EQ(status, Status::OK);
  EXPECT_FALSE(out.timings().empty());
  EXPECT_EQ(out.timings().size(), 42);
  EXPECT_EQ(out.timings()[0].mark_us, 889u);
  EXPECT_EQ(out.timings()[0].pause_us, 889u);
  EXPECT_EQ(out.timings()[25].mark_us, 889u);
  EXPECT_EQ(out.timings()[4].pause_us, 889u);
}

TEST_F(RealIrProtoTest, InvalidProtocolIndexReturnsError)
{
  Code code("0x0000F40101010020DF32CD010100");
  TimingStream out;
  // protocol index 99 doesn't exist
  auto status = file.serialiseIrStream(out, 99, code.getData());
  EXPECT_EQ(status, Status::ERROR_INDEX);
}
