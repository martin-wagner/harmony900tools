
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "decode.h"
#include "document/files/protocols.h"
#include "lib/bits.h"

using namespace std;
using namespace binary;
using namespace irProto;
using namespace document;

files::ProtocolCatalogue cat(TEST_CONFIG_PATH);


TEST(Decoder, Empty)
{
  auto dec = Decode(cat);
}

TEST(Decoder, NecHeader)
{
  vector<uint16_t> raw { 9032, 4437, 561, 550, 561, 1680 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::NEC);
}

TEST(Decoder, KASEIKYOHeader)
{
  vector<uint16_t> raw { 3300, 1700, 440, 400, 400, 1300 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::KASEIKYO);
}

TEST(Decoder, PhilipsRC5Header) //rc5
{
  vector<uint16_t> raw { 850, 860, 849, 855, 844 }; //manch 111, initial "false" not part of stream!
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
}

TEST(Decoder, PhilipsRC5HeaderRC5x1) //rc5x
{
  vector<uint16_t> raw { 1700, 860, 849, 855 }; //manch 100, initial "false" not part of stream!
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
}

TEST(Decoder, PhilipsRC5HeaderRC5x2) //rc5x
{
  vector<uint16_t> raw { 1700, 1658, 849 }; //manch 101, initial "false" not part of stream!
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
}

TEST(Decoder, PhilipsRC6Header)
{
  vector<uint16_t> raw { 2666, 889, 444, 444, 444, 444 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC6);
}

TEST(Decoder, SIRCSHeader)
{
  vector<uint16_t> raw { 2500, 550, 601, 550, 1222, 601 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::SIRCS);
}

TEST(Decoder, Samsung32Header)
{
  vector<uint16_t> raw { 4400, 4450, 550, 1600, 550, 550 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_EQ(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Samsung32);
}

TEST(Decoder, ShortHeader)
{
  vector<uint16_t> raw { 35, 4437, 561, 550, 561, 1680 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_NE(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, LongHeader)
{
  vector<uint16_t> raw { 15000, 4437, 561, 550, 561, 1680 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_NE(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, NoData)
{
  vector<uint16_t> raw {  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_NE(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, SinglePulse)
{
  //single pulse matching an actual protocol SoF will lead to a match on header
  //only check (because of lazy/tolerant matching)
  vector<uint16_t> raw { 767, 32000 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat);
  auto res = dec.parse(s, true);
  ASSERT_NE(res, Status::OK);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, NecData1)
{
  //Reference: 0x00FF00FF
  //Binary: 00000000 11111111 00000000 11111111
  vector<uint16_t> raw {
    9151, 4244,
    545, 541, 576, 572, 586, 532, 555, 528,
    541, 560, 528, 540, 570, 563, 541, 566,
    581, 1590, 581, 1730, 549, 1620, 591, 1657,
    533, 1608, 583, 1711, 581, 1737, 562, 1786,
    552, 563, 582, 568, 584, 565, 574, 529,
    542, 546, 532, 542, 533, 545, 569, 551,
    551, 1631, 544, 1779, 570, 1712, 538, 1736,
    537, 1666, 593, 1718, 564, 1727, 583, 1746,
    542, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  ASSERT_EQ(dec.getData().size(), 32);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x00FF00FF);
}

TEST(Decoder, NecData2)
{
  //Reference: 0x45BABA45
  //Binary: 01000101 10111010 10111010 01000101
  vector<uint16_t> raw {
      9460, 4688,
      538, 559, 541, 1670, 530, 552, 593, 544,
      579, 557, 555, 1783, 593, 564, 575, 1620,
      546, 1785, 565, 563, 577, 1600, 566, 1691,
      584, 1621, 591, 532, 539, 1709, 572, 542,
      534, 1769, 543, 566, 568, 1674, 566, 1695,
      589, 1630, 575, 542, 553, 1725, 547, 548,
      577, 531, 557, 1791, 593, 531, 541, 544,
      589, 586, 585, 1664, 537, 582, 574, 1713,
      593, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  ASSERT_EQ(dec.getData().size(), 32);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x45BABA45);
}

TEST(Decoder, NecData3)
{
  //Reference: 0x45BABA45
  //Binary: 01000101 10111010 10111010 01000101
  //same as test 3, just with non-jitter data
  vector<uint16_t> raw {
      9000, 4500,
      560, 560, 560, 1690, 560, 560, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 560,
      560, 1690, 560, 560, 560, 1690, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 1690,
      560, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  ASSERT_EQ(dec.getData().size(), 32);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x45BABA45);
}

TEST(Decoder, NecDataRepeat)
{
  //Reference: 0x45BABA45
  //Binary: 01000101 10111010 10111010 01000101 + repeat
  vector<uint16_t> raw {
      9000, 4500,
      560, 560, 560, 1690, 560, 560, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 560,
      560, 1690, 560, 560, 560, 1690, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 1690,
      560, 32000, 0, 32000, 9000, 2250, 560, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  ASSERT_EQ(dec.getData().size(), 32);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x45BABA45);
}

TEST(Decoder, NecDataShort)
{
  //Reference: 0x45
  //Binary: 01000101 10111010 10111010 (only 1 byte). fill with zeros
  vector<uint16_t> raw {
      9000, 4500,
      560, 560, 560, 1690, 560, 560, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 1690,
      560, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  ASSERT_EQ(dec.getData().size(), 32);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x45000000);
}






