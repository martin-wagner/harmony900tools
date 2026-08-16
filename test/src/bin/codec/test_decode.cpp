
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "decode.h"
#include "lib/bits.h"

using namespace std;
using namespace binary;
using namespace codec;
using namespace document;


TEST(Decoder, Empty)
{
  Decode();
}

TEST(Decoder, Short)
{
  vector<uint16_t> raw {
    9000, 4500
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode();
  auto res = dec.parse(s, 38000);
  ASSERT_EQ(res, Status::ERROR_SIZE);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, NoData)
{
  vector<uint16_t> raw {
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode();
  auto res = dec.parse(s, 38000);
  ASSERT_EQ(res, Status::ERROR_SIZE);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, SinglePulse)
{
  vector<uint16_t> raw { 9000, 32000 };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode();
  auto res = dec.parse(s, 38000);
  ASSERT_EQ(res, Status::ERROR_SIZE);
  EXPECT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, InvalidData)
{
  vector<uint16_t> raw {
    9151, 4244,
    545, 541, 576, 572, 586, 532, 555, 528,
    541, 560, 528, 540, 570, 563, 541, 566,
    581, 1590, 17, 1730, 549, 22, 591, 1657,
    533, 1608, 583, 1711, 581, 1737, 562, 1786,
    552, 563, 582, 568, 584, 565, 574, 529,
    542, 546, 532, 542, 533, 545, 569, 551,
    551, 1631, 544, 1779, 570, 1712, 538, 1736,
    537, 1666, 593, 1718, 564, 1727, 583, 1746,
    542, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 38000);
  ASSERT_NE(dec.getData().decoded, Status::OK);
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

  auto dec = Decode(s, 38000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 0);
  EXPECT_EQ(dec.getData().command, 0);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x00FF00FF);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
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

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 162);
  EXPECT_EQ(dec.getData().command, 93);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x45BABA45);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
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

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 162);
  EXPECT_EQ(dec.getData().command, 93);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x45BABA45);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, NecData1addr2cmd) //1x8 addr, 2x8 cmd
{
  //Binary: 00000000 11111111 01000000 11111110
  vector<uint16_t> raw {
    9000, 4500,
    560, 560, 560, 560, 560, 560, 560, 560,
    560, 560, 560, 560, 560, 560, 560, 560,
    560, 1690, 560, 1690, 560, 1690, 560, 1690,
    560, 1690, 560, 1690, 560, 1690, 560, 1690,
    560, 560, 560, 1690, 560, 560, 560, 560,
    560, 560, 560, 560, 560, 560, 560, 560,
    560, 1690, 560, 1690, 560, 1690, 560, 1690,
    560, 1690, 560, 1690, 560, 1690, 560, 560,
    560, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 255);
  EXPECT_EQ(dec.getData().command, 0x40fe);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x00FF40FE);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, NecDataExtNec) //2x8 addr, 1x8 cmd
{
  //Binary: 01000101 00111010 10111010 01000101
  vector<uint16_t> raw {
      9000, 4500,
      560, 560, 560, 1690, 560, 560, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 1690,
      560, 560, 560, 560, 560, 1690, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 560,
      560, 1690, 560, 560, 560, 1690, 560, 1690,
      560, 1690, 560, 560, 560, 1690, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 560,
      560, 560, 560, 1690, 560, 560, 560, 1690,
      560, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 23714);
  EXPECT_EQ(dec.getData().command, 93);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x453ABA45);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}


TEST(Decoder, NecDataOnkyo) //1x16 addr 1x16 cmd
{
  //Binary: 01000000 11111111 01000000 11111110
  vector<uint16_t> raw {
    9000, 4500,
    560, 560, 560, 1690, 560, 560, 560, 560,
    560, 560, 560, 560, 560, 560, 560, 560,
    560, 1690, 560, 1690, 560, 1690, 560, 1690,
    560, 1690, 560, 1690, 560, 1690, 560, 1690,
    560, 560, 560, 1690, 560, 560, 560, 560,
    560, 560, 560, 560, 560, 560, 560, 560,
    560, 1690, 560, 1690, 560, 1690, 560, 1690,
    560, 1690, 560, 1690, 560, 1690, 560, 560,
    560, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 0x40ff);
  EXPECT_EQ(dec.getData().command, 0x40fe);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x40FF40FE);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
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

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::NEC);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 162);
  EXPECT_EQ(dec.getData().command, 93);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x45BABA45);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
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

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::Unknown);
}

TEST(Decoder, KASEIKYOData1)
{
  //Reference: 0
  //Binary: 00000000 00000000 00000000 00000000 00000000 00000000
  vector<uint16_t> raw {
    3380, 1690, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 423, 423, 423, 423, 423,
    423, 423, 423, 32000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 38000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::KASEIKYO);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 0);
  EXPECT_EQ(dec.getData().command, 0);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
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

  auto dec = Decode(s, 38000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::KASEIKYO);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 17850);
  EXPECT_EQ(dec.getData().command, 3125084162);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x45BABA450002);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
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

  auto dec = Decode(s, 38000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::KASEIKYO);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 17850);
  EXPECT_EQ(dec.getData().command, 3125084162);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x45BABA450002);
  EXPECT_STREQ(dec.getData().error.c_str(), "");//ignore repeat
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

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 10);
  EXPECT_EQ(dec.getData().command, 42);
  EXPECT_EQ(dec.getData().data.size(), 12);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x2aa); //strip start
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, RC5Data2)
{
  //Reference: 0x11f0
  //Binary: 0000111110000 -- this forces rc5x
  vector<uint16_t> raw {
    1737,
    884, 889, 874, 911, 855, 866, 1853, 856, 900, 912, 856, 856, 856, 856, 900, 1900, 884, 889, 874, 911, 874, 911,
    31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 7);
  EXPECT_EQ(dec.getData().command, 112);
  EXPECT_EQ(dec.getData().data.size(), 13);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x11f0);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, RC5Data3)
{
  //Reference: 0x1E0F
  //Binary: 1111000001111
  vector<uint16_t> raw {
    889,
    889, 889, 889, 889, 889, 889, 889, 1778, 889, 889, 889, 889, 889, 889, 889, 889, 1778, 889, 889, 889, 889, 889, 889, 889,
    31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 24);
  EXPECT_EQ(dec.getData().command, 15);
  EXPECT_EQ(dec.getData().data.size(), 12);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0xE0F); //strip start
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, RC5Data4)
{
  //Reference: 0x1E8F
  //Binary: 1111010001111
  vector<uint16_t> raw {
    889,
    889, 889, 889, 889, 889, 889, 889, 1778, 1778, 1778, 889, 889, 889, 889, 1778, 889, 889, 889, 889, 889, 889, 889,
    31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC5);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 26);
  EXPECT_EQ(dec.getData().command, 15);
  EXPECT_EQ(dec.getData().data.size(), 12);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0xe8f); //strip start
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, RC6Data)
{
  //Binary: 1000 0 1010010000111010
  vector<uint16_t> raw {
    2666, 883, 444, 883, 444, 444, 444, 444, 444, 889, 1333, 888, 888,
    888, 444, 444, 444, 444, 444, 444, 888, 444, 444, 444, 444, 888,
    888, 888, 444, 31000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::PhilipsRC6);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 20);
  EXPECT_EQ(dec.getData().command, 58);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x143a);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, SIRCSdata)
{
  vector<uint16_t> raw {
      2400, 600,

      // command 18, LSB first: 0 1 0 0 1 0 0
      600, 600,
      1200, 600,
      600, 600,
      600, 600,
      1200, 600,
      600, 600,
      600, 600,

      // address 1, LSB first: 1 0 0 0 0
      1200, 600,
      600, 600,
      600, 600,
      600, 600,
      600, 25000
  };
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 40000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::SIRCS12);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 1);
  EXPECT_EQ(dec.getData().command, 18);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0x490);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}

TEST(Decoder, Samsung32data)
{
  vector<uint16_t> raw {
    4500, 4500,

    // 0x07, LSB first: 1 1 1 0 0 0 0 0
    550, 1650,
    550, 1650,
    550, 1650,
    550, 550,
    550, 550,
    550, 550,
    550, 550,
    550, 550,

    // 0x07, LSB first: 1 1 1 0 0 0 0 0
    550, 1650,
    550, 1650,
    550, 1650,
    550, 550,
    550, 550,
    550, 550,
    550, 550,
    550, 550,

    // 0xE0, LSB first: 0 0 0 0 0 1 1 1
    550, 550,
    550, 550,
    550, 550,
    550, 550,
    550, 550,
    550, 1650,
    550, 1650,
    550, 1650,

    // 0x1F, LSB first: 1 1 1 1 1 0 0 0
    550, 1650,
    550, 1650,
    550, 1650,
    550, 1650,
    550, 1650,
    550, 550,
    550, 550,
    550, 550,

    550, 32000
};
  auto s = TimingStream::fromMarkPause(raw);

  auto dec = Decode(s, 36000);
  ASSERT_EQ(dec.getData().decoded, Status::OK);
  ASSERT_EQ(dec.getCodeType(), data::CodeType::Samsung32);
  EXPECT_STRNE(dec.getData().codeString.c_str(), "");
  EXPECT_EQ(dec.getData().address, 0x07);
  EXPECT_EQ(dec.getData().command, 0xE0);
  EXPECT_EQ(lib::bitsTou64Msb(dec.getData().data), 0xE0E007F8);
  EXPECT_STREQ(dec.getData().error.c_str(), "");
}
