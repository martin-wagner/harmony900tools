
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

TEST(Decoder, KASEIKYOData1)
{
  //Reference: 0x0008FB040000
  //Binary: 00000000 00001000 11111011 00000100 00000000 00000000
  vector<uint16_t> raw {
    3458, 1736,
    424, 403, 437, 436, 429, 404, 427, 431,
    407, 408, 413, 404, 408, 440, 412, 429,
    429, 423, 421, 418, 424, 442, 443, 436,
    417, 1280, 405, 420, 438, 418, 404, 407,
    438, 1219, 403, 1322, 415, 1246, 412, 1215,
    428, 1265, 436, 443, 405, 1313, 406, 1328,
    419, 440, 420, 436, 424, 406, 439, 409,
    415, 403, 439, 1315, 442, 416, 431, 441,
    431, 432, 411, 405, 421, 417, 436, 404,
    427, 414, 419, 410, 427, 426, 403, 418,
    431, 407, 439, 428, 403, 407, 436, 441,
    433, 428, 403, 406, 419, 417, 407, 406,
    427, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::KASEIKYO);
  ASSERT_EQ(dec.getData().size(), 48);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x0008FB040000);
}

TEST(Decoder, KASEIKYOData2)
{
  //Reference: 0x45BABA450002
  //Binary: 01000101 10111010 10111010 01000101 00000000 00000010
  vector<uint16_t> raw {
    3237, 1764,
    425, 420, 408, 1208, 425, 412, 441, 413,
    424, 426, 428, 1210, 419, 431, 433, 1228,
    436, 1239, 412, 411, 410, 1303, 417, 1235,
    403, 1294, 428, 439, 403, 1304, 435, 428,
    426, 1240, 408, 433, 413, 1277, 435, 1224,
    425, 1206, 428, 443, 435, 1310, 420, 427,
    417, 415, 425, 1227, 429, 422, 424, 411,
    406, 432, 441, 1250, 441, 432, 440, 1294,
    423, 431, 440, 439, 422, 405, 427, 425,
    408, 417, 408, 424, 433, 413, 439, 430,
    405, 432, 432, 443, 409, 426, 434, 404,
    409, 439, 421, 424, 433, 1269, 425, 406,
    417, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::KASEIKYO);
  ASSERT_EQ(dec.getData().size(), 48);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x45BABA450002);
}

TEST(Decoder, KASEIKYODataRepeat)
{
  //Reference: 0x45BABA450002
  //Binary: 01000101 10111010 10111010 01000101 00000000 00000010 + twice (repeat)
  vector<uint16_t> raw {
    3237, 1764,
    425, 420, 408, 1208, 425, 412, 441, 413,
    424, 426, 428, 1210, 419, 431, 433, 1228,
    436, 1239, 412, 411, 410, 1303, 417, 1235,
    403, 1294, 428, 439, 403, 1304, 435, 428,
    426, 1240, 408, 433, 413, 1277, 435, 1224,
    425, 1206, 428, 443, 435, 1310, 420, 427,
    417, 415, 425, 1227, 429, 422, 424, 411,
    406, 432, 441, 1250, 441, 432, 440, 1294,
    423, 431, 440, 439, 422, 405, 427, 425,
    408, 417, 408, 424, 433, 413, 439, 430,
    405, 432, 432, 443, 409, 426, 434, 404,
    409, 439, 421, 424, 433, 1269, 425, 406,
    417, 32000, 0, 32000, 0, 14000,
    3237, 1764,
    425, 420, 408, 1208, 425, 412, 441, 413,
    424, 426, 428, 1210, 419, 431, 433, 1228,
    436, 1239, 412, 411, 410, 1303, 417, 1235,
    403, 1294, 428, 439, 403, 1304, 435, 428,
    426, 1240, 408, 433, 413, 1277, 435, 1224,
    425, 1206, 428, 443, 435, 1310, 420, 427,
    417, 415, 425, 1227, 429, 422, 424, 411,
    406, 432, 441, 1250, 441, 432, 440, 1294,
    423, 431, 440, 439, 422, 405, 427, 425,
    408, 417, 408, 424, 433, 413, 439, 430,
    405, 432, 432, 443, 409, 426, 434, 404,
    409, 439, 421, 424, 433, 1269, 425, 406,
    417, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::KASEIKYO);
  ASSERT_EQ(dec.getData().size(), 48);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x45BABA450002); //ignore repeat
}

TEST(Decoder, RC5Data1)
{
  //Reference: 0x12aa
  //Binary: 1001010101010
  vector<uint16_t> raw {
    865,
    884, 1737, 874, 911, 1853, 1851, 1800, 1758, 1740, 1866, 1854, 1812, 1847, 1708,
    31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  ASSERT_EQ(dec.getData().size(), 13);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x12aa);
}

TEST(Decoder, RC5Data2)
{
  //Reference: 0x12aa
  //Binary: 0000111110000
  vector<uint16_t> raw {
    1737,
    884, 889, 874, 911, 855, 866, 1853, 856, 900, 912, 856, 856, 856, 856, 900, 1900, 884, 889, 874, 911, 874, 911,
    31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  ASSERT_EQ(dec.getData().size(), 13);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x1f0);
}

TEST(Decoder, RC5Data3)
{
  //Reference: 0x12aa
  //Binary: 1111000001111
  vector<uint16_t> raw {
    889,
    889, 889, 889, 889, 889, 889, 889, 1778, 889, 889, 889, 889, 889, 889, 889, 889, 1778, 889, 889, 889, 889, 889, 889, 889,
    31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  ASSERT_EQ(dec.getData().size(), 13);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x1E0F);
}

TEST(Decoder, RC5Data4)
{
  //Reference: 0x12aa
  //Binary: 1111010001111
  vector<uint16_t> raw {
    889,
    889, 889, 889, 889, 889, 889, 889, 1778, 1778, 1778, 889, 889, 889, 889, 1778, 889, 889, 889, 889, 889, 889, 889,
    31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  ASSERT_EQ(dec.getData().size(), 13);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()), 0x1E0F);
}

TEST(Decoder, RC6Data1)
{
  //Reference: 0xA43A
  //Binary: 0000 0 1010010000111010
  vector<uint16_t> raw {
    2666, 1333, 444, 444, 444, 444, 444, 444, 444, 889, 1333, 888, 888,
    888, 444, 444, 444, 444, 444, 444, 888, 444, 444, 444, 444, 888,
    888, 888, 444, 31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(cat, s);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC6);
  ASSERT_EQ(dec.getData().size(), 21);
  EXPECT_EQ(lib::bitsTou64Lsb(dec.getData()),0xA43A);
}
