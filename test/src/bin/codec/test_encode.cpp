#include <gtest/gtest.h>

#include "encode.h"
#include "bin/irProto/code.h"

using namespace std;
using namespace binary;
using namespace codec;

namespace
{

uint64_t singleSectionData(const irProto::Code &code)
{
  return irProto::Code::bitsToU64(code.accessSections().at(0).getData());
}

}

TEST(Encoder, NecData1)
{
  //address 0, command 0 -> 0x00FF00FF (verified against Decoder.NecData1)
  auto code = encodeNec(0, "NEC", 0, 0, vector<bool>(32));

  ASSERT_EQ(code.getControl(), static_cast<uint8_t>(irProto::Code::Ctrl::FLAT));
  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x00FF00FFULL);
  EXPECT_EQ(code.getRepeatFrame(), 1);
}

TEST(Encoder, NecData2)
{
  //address 162, command 93 -> 0x45BABA45 (verified against Decoder.NecData2)
  auto code = encodeNec(0, "NEC", 162, 93, vector<bool>(32));

  ASSERT_EQ(code.getControl(), static_cast<uint8_t>(irProto::Code::Ctrl::FLAT));
  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x45BABA45ULL);
  EXPECT_EQ(code.getRepeatFrame(), 1);
}

TEST(Encoder, NecLikeData)
{
  //NEC_LIKE packs address/command directly, 16 bits each, no inversion
  auto code = encodeNec(0, "NEC_LIKE", 0x00ff, 0x40fe, vector<bool>(32));

  ASSERT_EQ(code.getControl(), static_cast<uint8_t>(irProto::Code::Ctrl::FLAT));
  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x00FF40FEULL);
  EXPECT_EQ(code.getRepeatFrame(), 1);
}

TEST(Encoder, KaseikyoPanasonic)
{
  //device/subdevice/function come from bits 8-31 of rawData (top byte unused),
  //manufacturer is the address param, checksum is device ^ subdevice ^ function
  const uint32_t manufacturer = 0x45ba;
  const uint8_t device = 0xba;
  const uint8_t subdevice = 0x45;
  const uint8_t function = 0x02;
  const uint64_t rawValue = (static_cast<uint64_t>(device) << 24)
      | (static_cast<uint64_t>(subdevice) << 16)
      | (static_cast<uint64_t>(function) << 8);
  auto rawData = irProto::Code::u64tobits(48, rawValue);
  auto code = encodeKASEIKYO(0, "PANASONIC", manufacturer, 0, rawData);

  const uint8_t checksum = device ^ subdevice ^ function;
  const uint64_t expected = (static_cast<uint64_t>(manufacturer) << 32)
      | (static_cast<uint64_t>(device) << 24)
      | (static_cast<uint64_t>(subdevice) << 16)
      | (static_cast<uint64_t>(function) << 8) | checksum;

  ASSERT_EQ(code.getControl(),
      static_cast<uint8_t>(irProto::Code::Ctrl::SECTIONS_1));
  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), expected);
  EXPECT_EQ(code.getRepeatFrame(), 0);
  EXPECT_EQ(code.getDataFrameTxCount(), irProto::Code::DEFAULT_REPEATS);
}

TEST(Encoder, KaseikyoZero)
{
  auto rawData = irProto::Code::u64tobits(48, 0);
  auto code = encodeKASEIKYO(0, "PANASONIC", 0, 0, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0ULL);
}

TEST(Encoder, SIRCS12)
{
  //address 1, command 18 -> 0x490 (verified against Decoder.SIRCS12data)
  auto rawData = irProto::Code::u64tobits(12, 0);
  auto code = encodeSIRCS(0, "SONY", 1, 18, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x490ULL);
  EXPECT_EQ(code.getRepeatFrame(), 0);
  EXPECT_EQ(code.getDataFrameTxCount(), irProto::Code::DEFAULT_REPEATS);
}

TEST(Encoder, SIRCS15)
{
  //nbits=15 -> 8 address bits, 7 command bits, then reversed over 15 bits.
  //address 129, command 18 -> 0x2481
  auto rawData = irProto::Code::u64tobits(15, 0);
  auto code = encodeSIRCS(0, "SONY", 129, 18, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x2481ULL);
}

TEST(Encoder, SIRCS20)
{
  //encodeSIRCS20 splits the command param into a 7-bit command (low bits) and
  //an 8-bit extended value (bits 7+), matching IRsend::encodeSony(20, ...).
  //address 5, command 200 -> 7-bit cmd 0x48, extended 0x01 -> 0x13480
  auto rawData = irProto::Code::u64tobits(20, 0);
  auto code = encodeSIRCS20(0, "SONY", 5, 200, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x13480ULL);
}

TEST(Encoder, Samsung32)
{
  //address 0x07, command 0xE0 -> 0xE0E007F8 (verified against Decoder.Samsung32data)
  auto rawData = irProto::Code::u64tobits(32, 0);
  auto code = encodeSamsung32(0, "SAMSUNG", 0x07, 0xE0, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0xE0E007F8ULL);
  EXPECT_EQ(code.getRepeatFrame(), 0);
  EXPECT_EQ(code.getDataFrameTxCount(), irProto::Code::DEFAULT_REPEATS);
}

TEST(Encoder, PhilipsRC5)
{
  //address 10, command 42 -> 0x2aa data bits, with 2nd start bit forced set
  //(verified against Decoder.RC5Data1, which strips the start bit)
  auto rawData = irProto::Code::u64tobits(13, 0);
  auto code = encodePhilipsRC5(0, "RC5", 10, 42, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code) & 0xFFFULL, 0x2aaULL);
  EXPECT_EQ(code.getRepeatFrame(), 0);
  EXPECT_EQ(code.getDataFrameTxCount(), irProto::Code::DEFAULT_REPEATS);
}

TEST(Encoder, PhilipsRC5X)
{
  //address 7, command 112 -> 0x11f0 (verified against Decoder.RC5Data2)
  auto rawData = irProto::Code::u64tobits(13, 0);
  auto code = encodePhilipsRC5(0, "RC5X", 7, 112, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x11f0ULL);
}

TEST(Encoder, PhilipsRC6)
{
  //address 20, command 58 -> data payload 0x143a (verified against Decoder.RC6Data)
  auto rawData = irProto::Code::u64tobits(16, 0);
  auto code = encodePhilipsRC6(0, "RC6", 20, 58, rawData);

  //createMultiSection sets ctrl to the actual section count (SECTIONS_N is a
  //placeholder for "2 or more", the real value tracks how many sections exist)
  ASSERT_EQ(code.getControl(), 3);
  ASSERT_EQ(code.getDataSectionCount(), 3);
  EXPECT_EQ(irProto::Code::bitsToU64(code.accessSections().at(0).getData()),
      0x8001ULL);
  EXPECT_EQ(irProto::Code::bitsToU64(code.accessSections().at(1).getData()),
      0x0002ULL);
  EXPECT_EQ(irProto::Code::bitsToU64(code.accessSections().at(2).getData()),
      0x143aULL);
  EXPECT_EQ(code.getRepeatFrame(), 0);
  EXPECT_EQ(code.getDataFrameTxCount(), irProto::Code::DEFAULT_REPEATS);
}

TEST(Encoder, PhilipsRC6A)
{
  //address & 0xFFFFFFF, command in low byte
  auto rawData = irProto::Code::u64tobits(31, 0);
  auto code = encodePhilipsRC6A(0, "RC6", 0x1234, 0x56, rawData);

  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), (0x1234ULL << 8) | 0x56ULL);
  EXPECT_EQ(code.getRepeatFrame(), 0);
  EXPECT_EQ(code.getDataFrameTxCount(), irProto::Code::DEFAULT_REPEATS);
}

TEST(Encoder, DefaultsNec)
{
  string codeString;
  uint32_t address, command;
  vector<bool> rawData;

  auto code = encodeDefaults(0, document::data::CodeType::NEC, codeString,
      address, command, rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 32);
  EXPECT_STREQ(codeString.c_str(), "NEC");
  ASSERT_EQ(code.getControl(), static_cast<uint8_t>(irProto::Code::Ctrl::FLAT));
  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0x00FF00FFULL);
}

TEST(Encoder, DefaultsKaseikyo)
{
  string codeString;
  uint32_t address, command;
  vector<bool> rawData;

  auto code = encodeDefaults(0, document::data::CodeType::KASEIKYO, codeString,
      address, command, rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 48);
  EXPECT_STREQ(codeString.c_str(), "PANASONIC");
  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0ULL);
}

TEST(Encoder, DefaultsSamsung32)
{
  string codeString;
  uint32_t address, command;
  vector<bool> rawData;

  auto code = encodeDefaults(0, document::data::CodeType::Samsung32, codeString,
      address, command, rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 32);
  EXPECT_STREQ(codeString.c_str(), "SAMSUNG");
  ASSERT_EQ(code.getDataSectionCount(), 1);
  EXPECT_EQ(singleSectionData(code), 0xFFULL); //addr 0, cmd 0 -> encodeSAMSUNG(0,0)
}

TEST(Encoder, DefaultsPhilipsRC6)
{
  string codeString;
  uint32_t address, command;
  vector<bool> rawData;

  auto code = encodeDefaults(0, document::data::CodeType::PhilipsRC6,
      codeString, address, command, rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 16);
  EXPECT_STREQ(codeString.c_str(), "RC6");
  ASSERT_EQ(code.getDataSectionCount(), 3);
}

TEST(Encoder, DefaultsUnknown)
{
  string codeString;
  uint32_t address, command;
  vector<bool> rawData;

  auto code = encodeDefaults(0, document::data::CodeType::Unknown, codeString,
      address, command, rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_TRUE(rawData.empty());
  EXPECT_STREQ(codeString.c_str(), "");
}

TEST(Encoder, GetCodeSizeNec)
{
  uint32_t addressWidth, commandWidth;

  ASSERT_TRUE(
      getCodeSize(document::data::CodeType::NEC, string("NEC"), addressWidth,
          commandWidth));
  EXPECT_EQ(addressWidth, 16);
  EXPECT_EQ(commandWidth, 8);

  ASSERT_TRUE(
      getCodeSize(document::data::CodeType::NEC, string("NEC_LIKE"),
          addressWidth, commandWidth));
  EXPECT_EQ(addressWidth, 16);
  EXPECT_EQ(commandWidth, 16);
}

TEST(Encoder, GetCodeSizeRC5)
{
  uint32_t addressWidth, commandWidth;

  ASSERT_TRUE(
      getCodeSize(document::data::CodeType::PhilipsRC5, string("RC5"),
          addressWidth, commandWidth));
  EXPECT_EQ(addressWidth, 5);
  EXPECT_EQ(commandWidth, 6);

  ASSERT_TRUE(
      getCodeSize(document::data::CodeType::PhilipsRC5, string("RC5X"),
          addressWidth, commandWidth));
  EXPECT_EQ(addressWidth, 5);
  EXPECT_EQ(commandWidth, 7);
}

TEST(Encoder, GetCodeSizeUnknownCodeString)
{
  uint32_t addressWidth, commandWidth;

  //NEC codeType with a codeString that isn't NEC/NEC_LIKE -- no size known
  EXPECT_FALSE(
      getCodeSize(document::data::CodeType::NEC, string("SONY"), addressWidth,
          commandWidth));
  EXPECT_EQ(addressWidth, 0);
  EXPECT_EQ(commandWidth, 0);
}

TEST(Encoder, GetCodeSizeKaseikyoUnsupported)
{
  uint32_t addressWidth, commandWidth;

  //KASEIKYO has no simple address/command width
  EXPECT_FALSE(
      getCodeSize(document::data::CodeType::KASEIKYO, string("PANASONIC"),
          addressWidth, commandWidth));
}

TEST(Encoder, GetSubTypesNec)
{
  auto types = getSubTypes(document::data::CodeType::NEC);

  ASSERT_EQ(types.size(), 2);
  EXPECT_EQ(types.at(0), QString("NEC"));
  EXPECT_EQ(types.at(1), QString("NEC_LIKE"));
}

TEST(Encoder, GetSubTypesKaseikyo)
{
  auto types = getSubTypes(document::data::CodeType::KASEIKYO);

  ASSERT_EQ(types.size(), 2);
  EXPECT_EQ(types.at(0), QString("PANASONIC"));
  EXPECT_EQ(types.at(1), QString("DENON"));
}

TEST(Encoder, GetSubTypesUnknown)
{
  auto types = getSubTypes(document::data::CodeType::Unknown);

  EXPECT_TRUE(types.empty());
}

TEST(Encoder, EncodeDispatchMatchesDirectCall)
{
  //the encode() dispatcher in encode.h must route to the same function/result
  //as calling encodeNec() directly
  auto rawData = vector<bool>(32);
  auto direct = encodeNec(0, "NEC", 162, 93, rawData);
  auto dispatched = encode(0, document::data::CodeType::NEC, "NEC", 162, 93,
      rawData);

  EXPECT_EQ(direct.getControl(), dispatched.getControl());
  EXPECT_EQ(direct.getRepeatFrame(), dispatched.getRepeatFrame());
}
