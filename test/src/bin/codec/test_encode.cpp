#include <gtest/gtest.h>

#include "encode.h"
#include "bin/irProto/code.h"

using namespace std;
using namespace binary;
using namespace irProto;
using namespace codec;

namespace
{

/** section with <symbols> symbols and <quanta> states per symbol (0 = no payload) */
TimingSection makeSection(uint16_t symbols, uint8_t quanta)
{
  TimingSectionIrPayload payload;

  for (uint8_t q = 0; q < quanta; q++) {
    std::vector<Item> timings = { { true, 560 }, {
      false,
      static_cast<uint16_t>(560 * (q + 1)) } };
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
  return makeProto(
      { makeSection(4, 2), makeSection(1, 2), makeSection(16, 2) });
}

}

TEST(Encoder, NecData1)
{
  //address 0, command 0 -> 0x00FF00FF (verified against Decoder.NecData1)
  auto code = encodeNec(0, protoNec(), "NEC", 0, 0, vector<uint8_t>(32));

  ASSERT_EQ(code.accessStartGroups().size(), 1);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessStartGroups().at(0).getData());
  EXPECT_EQ(data, 0x00FF00FFULL);
}

TEST(Encoder, NecData2)
{
  //address 162, command 93 -> 0x45BABA45 (verified against Decoder.NecData2)
  auto code = encodeNec(0, protoNec(), "NEC", 162, 93, vector<uint8_t>(32));

  ASSERT_EQ(code.accessStartGroups().size(), 1);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessStartGroups().at(0).getData());
  EXPECT_EQ(data, 0x45BABA45ULL);
}

TEST(Encoder, NecLikeData)
{
  //NEC_LIKE packs address/command directly, 16 bits each, no inversion
  auto code = encodeNec(0, protoNec(), "NEC_LIKE", 0x00ff, 0x40fe,
      vector<uint8_t>(32));

  ASSERT_EQ(code.accessStartGroups().size(), 1);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessStartGroups().at(0).getData());
  EXPECT_EQ(data, 0x00FF40FEULL);
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
  auto rawData = irProto::Code::u64bitsToList(48, rawValue);
  auto code = encodeKASEIKYO(0, protoSingle(48), "PANASONIC", manufacturer, 0,
      rawData);

  const uint8_t checksum = device ^ subdevice ^ function;
  const uint64_t expected = (static_cast<uint64_t>(manufacturer) << 32)
      | (static_cast<uint64_t>(device) << 24)
      | (static_cast<uint64_t>(subdevice) << 16)
      | (static_cast<uint64_t>(function) << 8) | checksum;

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, expected);
}

TEST(Encoder, KaseikyoZero)
{
  auto rawData = irProto::Code::u64bitsToList(48, 0);
  auto code = encodeKASEIKYO(0, protoSingle(48), "PANASONIC", 0, 0, rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0);
}

TEST(Encoder, SIRCS12)
{
  //address 1, command 18 -> 0x490 (verified against Decoder.SIRCS12data)
  auto rawData = irProto::Code::u64bitsToList(12, 0);
  auto code = encodeSIRCS(0, protoSingle(12), "SONY", 1, 18, rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0x490ULL);
}

TEST(Encoder, SIRCS15)
{
  //nbits=15 -> 8 address bits, 7 command bits, then reversed over 15 bits.
  //address 129, command 18 -> 0x2481
  auto rawData = irProto::Code::u64bitsToList(15, 0);
  auto code = encodeSIRCS(0, protoSingle(15), "SONY", 129, 18, rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0x2481ULL);
}

TEST(Encoder, SIRCS20)
{
  //encodeSIRCS20 splits the command param into a 7-bit command (low bits) and
  //an 8-bit extended value (bits 7+), matching IRsend::encodeSony(20, ...).
  //address 5, command 200 -> 7-bit cmd 0x48, extended 0x01 -> 0x13480
  auto rawData = irProto::Code::u64bitsToList(20, 0);
  auto code = encodeSIRCS20(0, protoSingle(20), "SONY", 5, 200, rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0x13480ULL);
}

TEST(Encoder, Samsung32)
{
  //address 0x07, command 0xE0 -> 0xE0E007F8 (verified against Decoder.Samsung32data)
  auto rawData = irProto::Code::u64bitsToList(32, 0);
  auto code = encodeSamsung32(0, protoSingle(32), "SAMSUNG", 0x07, 0xE0,
      rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0xE0E007F8ULL);
}

TEST(Encoder, PhilipsRC5)
{
  //address 10, command 42 -> 0x2aa data bits, with 2nd start bit forced set
  //(verified against Decoder.RC5Data1, which strips the start bit)
  auto rawData = irProto::Code::u64bitsToList(13, 0);
  auto code = encodePhilipsRC5(0, protoSingle(13), "RC5", 10, 42, rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data & 0xFFFULL, 0x2aaULL);
}

TEST(Encoder, PhilipsRC5X)
{
  //address 7, command 112 -> 0x11f0 (verified against Decoder.RC5Data2)
  auto rawData = irProto::Code::u64bitsToList(13, 0);
  auto code = encodePhilipsRC5(0, protoSingle(13), "RC5X", 7, 112, rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0x11f0ULL);
}

TEST(Encoder, PhilipsRC6)
{
  //address 20, command 58 -> data payload 0x143a (verified against Decoder.RC6Data)
  auto rawData = irProto::Code::u64bitsToList(16, 0);
  auto code = encodePhilipsRC6(0, protoRc6(), "RC6", 20, 58, rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 3);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0x08);
  data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(1).getData());
  EXPECT_EQ(data, 0x00);
  data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(2).getData());
  EXPECT_EQ(data, 0x143aULL);
}

TEST(Encoder, PhilipsRC6A)
{
  //address & 0xFFFFFFF, command in low byte
  auto rawData = irProto::Code::u64bitsToList(31, 0);
  auto code = encodePhilipsRC6A(0, protoSingle(31), "RC6", 0x1234, 0x56,
      rawData);

  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, (0x1234ULL << 8) | 0x56ULL);
}

TEST(Encoder, DefaultsNec)
{
  string codeString;
  uint32_t address, command;
  vector<uint8_t> rawData;

  auto code = encodeDefaults(0, protoNec(), document::data::CodeType::NEC,
      codeString, address, command, rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 32);
  EXPECT_STREQ(codeString.c_str(), "NEC");
  ASSERT_EQ(code.accessStartGroups().size(), 1);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessStartGroups().at(0).getData());
  EXPECT_EQ(data, 0x00FF00FFULL);
}

TEST(Encoder, DefaultsKaseikyo)
{
  string codeString;
  uint32_t address, command;
  vector<uint8_t> rawData;

  auto code = encodeDefaults(0, protoSingle(48),
      document::data::CodeType::KASEIKYO, codeString, address, command,
      rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 48);
  EXPECT_STREQ(codeString.c_str(), "PANASONIC");
  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0ULL);
}

TEST(Encoder, DefaultsSamsung32)
{
  string codeString;
  uint32_t address, command;
  vector<uint8_t> rawData;

  auto code = encodeDefaults(0, protoSingle(32),
      document::data::CodeType::Samsung32, codeString, address, command,
      rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 32);
  EXPECT_STREQ(codeString.c_str(), "SAMSUNG");
  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 1);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
  auto data = irProto::Code::listToU64bits(
      code.accessRepeatGroups().at(0).getData());
  EXPECT_EQ(data, 0xFFULL);
}

TEST(Encoder, DefaultsPhilipsRC6)
{
  string codeString;
  uint32_t address, command;
  vector<uint8_t> rawData;

  auto code = encodeDefaults(0, protoRc6(),
      document::data::CodeType::PhilipsRC6, codeString, address, command,
      rawData);

  EXPECT_EQ(address, 0);
  EXPECT_EQ(command, 0);
  EXPECT_EQ(rawData.size(), 16);
  EXPECT_STREQ(codeString.c_str(), "RC6");
  ASSERT_EQ(code.accessStartGroups().size(), 0);
  ASSERT_EQ(code.accessRepeatGroups().size(), 3);
  ASSERT_EQ(code.accessFinishGroups().size(), 0);
}

TEST(Encoder, DefaultsUnknown)
{
  string codeString;
  uint32_t address, command;
  vector<uint8_t> rawData;

  auto code = encodeDefaults(0, protoSingle(16),
      document::data::CodeType::Unknown, codeString, address, command, rawData);

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
  auto rawData = vector<uint8_t>(32);
  auto direct = encodeNec(0, protoNec(), "NEC", 162, 93, rawData);
  auto dispatched = encode(0, protoNec(), document::data::CodeType::NEC, "NEC",
      162, 93, rawData);

  EXPECT_EQ(direct.serialiseVec(), dispatched.serialiseVec());
}
