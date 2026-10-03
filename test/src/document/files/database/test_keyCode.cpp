// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>

#include <QFile>
#include <QTextStream>

#include "keyCode.h"

using namespace document::files;

// ===========================================================================
// Static examples
// ===========================================================================

class KeyCodeTest: public ::testing::Test
{
  protected:
    void expectNumber(const QVariant &value, int prefix, const QString &digits)
    {
      ASSERT_EQ(value.userType(), qMetaTypeId<KeyCodeNumber>());
      const KeyCodeNumber number = value.value<KeyCodeNumber>();
      EXPECT_EQ(number.prefix, prefix);
      EXPECT_EQ(number.digits, digits);
    }

    void expectText(const QVariant &value, const QString &text)
    {
      ASSERT_EQ(value.userType(), QMetaType::QString);
      EXPECT_EQ(value.toString(), text);
    }
};

TEST_F(KeyCodeTest, PhilipsRC6A)
{
  const QString text = "G:PhilipsRC6A 16 Bit LongToggle:()(0x7_1x0_2xB9FE)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getGroup(), "G");
  EXPECT_EQ(result.getName(), "PhilipsRC6A 16 Bit LongToggle");
  EXPECT_EQ(result.getBitCount(), 16);
  EXPECT_EQ(result.getTrailer(), 3);
  EXPECT_EQ(result.getItemType(0), KeyCode::ItemType::Empty);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Array);
  const QVariantList values = result.getItem(1).toList();
  ASSERT_EQ(values.size(), 3);
  expectNumber(values[0], 0, "7");
  expectNumber(values[1], 1, "0");
  expectNumber(values[2], 2, "B9FE");
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, SIRCS15Bit)
{
  const QString text = "G:SIRCS 15 Bit:()(0x5CE9)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getGroup(), "G");
  EXPECT_EQ(result.getName(), "SIRCS 15 Bit");
  EXPECT_EQ(result.getBitCount(), 15);
  EXPECT_EQ(result.getTrailer(), 3);
  EXPECT_EQ(result.getItemType(0), KeyCode::ItemType::Empty);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Number);
  expectNumber(result.getItem(1), 0, "5CE9");
  uint64_t value = 0;
  EXPECT_TRUE(result.getItem(1).value<KeyCodeNumber>().toUInt64(value));
  EXPECT_EQ(value, 0x5CE9u);
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, KmX12Bit)
{
  const QString text = "G:KmX 12 Bit:()(0xD10)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getGroup(), "G");
  EXPECT_EQ(result.getName(), "KmX 12 Bit");
  EXPECT_EQ(result.getBitCount(), 12);
  EXPECT_EQ(result.getTrailer(), 3);
  EXPECT_EQ(result.getItemType(0), KeyCode::ItemType::Empty);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Number);
  expectNumber(result.getItem(1), 0, "D10");
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, KASEIKYO32Bit)
{
  const QString text = "G:KASEIKYO 32 Bit:(0x20DFDA25)(Repeat)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getGroup(), "G");
  EXPECT_EQ(result.getName(), "KASEIKYO 32 Bit");
  EXPECT_EQ(result.getBitCount(), 32);
  EXPECT_EQ(result.getTrailer(), 3);
  ASSERT_EQ(result.getItemType(0), KeyCode::ItemType::Number);
  expectNumber(result.getItem(0), 0, "20DFDA25");
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Text);
  expectText(result.getItem(1), "Repeat");
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, Ms30Bit)
{
  const QString text = "G:Ms 30 Bit:()(0x3FAD6FA6)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getGroup(), "G");
  EXPECT_EQ(result.getName(), "Ms 30 Bit");
  EXPECT_EQ(result.getBitCount(), 30);
  EXPECT_EQ(result.getTrailer(), 3);
  EXPECT_EQ(result.getItemType(0), KeyCode::ItemType::Empty);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Number);
  expectNumber(result.getItem(1), 0, "3FAD6FA6");
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, Sha15Bit)
{
  const QString text = "G:Sha 15 Bit:()(0x47BE_0x4441)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getGroup(), "G");
  EXPECT_EQ(result.getName(), "Sha 15 Bit");
  EXPECT_EQ(result.getBitCount(), 15);
  EXPECT_EQ(result.getTrailer(), 3);
  EXPECT_EQ(result.getItemType(0), KeyCode::ItemType::Empty);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Array);
  const QVariantList values = result.getItem(1).toList();
  ASSERT_EQ(values.size(), 2);
  expectNumber(values[0], 0, "47BE");
  expectNumber(values[1], 0, "4441");
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, ArrayWithWordsAndNumbers)
{
  const QString text = "G:Dual 8 16 Bit:()(Start_0x28_0x68_Trailer)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getName(), "Dual 8 16 Bit");
  EXPECT_EQ(result.getBitCount(), 16);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Array);
  const QVariantList values = result.getItem(1).toList();
  ASSERT_EQ(values.size(), 4);
  expectText(values[0], "Start");
  expectNumber(values[1], 0, "28");
  expectNumber(values[2], 0, "68");
  expectText(values[3], "Trailer");
}

TEST_F(KeyCodeTest, TextInThirdItem)
{
  const QString text = "G:Concept 10 Bit:(Start)(0x00F)(Finish):3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getBitCount(), 10);
  expectText(result.getItem(0), "Start");
  expectNumber(result.getItem(1), 0, "00F");
  expectText(result.getItem(2), "Finish");
}

TEST_F(KeyCodeTest, AllThreeItemsUsed)
{
  const QString text =
      "G:Fujikura 32 Bit:(0xFAFA17E8)(0xFAFA15EA)(0xFAFA14EB):3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getItemType(0), KeyCode::ItemType::Number);
  EXPECT_EQ(result.getItemType(1), KeyCode::ItemType::Number);
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Number);
  expectNumber(result.getItem(0), 0, "FAFA17E8");
  expectNumber(result.getItem(1), 0, "FAFA15EA");
  expectNumber(result.getItem(2), 0, "FAFA14EB");
}

TEST_F(KeyCodeTest, PrefixedSingleNumber)
{
  const QString text = "G:Vx 33 Bit:(0x1FC000729)(1x38)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  expectNumber(result.getItem(0), 0, "1FC000729");
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Number);
  expectNumber(result.getItem(1), 1, "38");
}

TEST_F(KeyCodeTest, NumberWiderThan64Bit)
{
  const QString text = "G:Nat 152 Bit:(1x4004072000000060)"
      "(0x4004072000001401CC000060060800010060BB)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getBitCount(), 152);
  expectNumber(result.getItem(0), 1, "4004072000000060");
  expectNumber(result.getItem(1), 0, "4004072000001401CC000060060800010060BB");
  uint64_t value = 0;
  EXPECT_FALSE(result.getItem(1).value<KeyCodeNumber>().toUInt64(value));
}

TEST_F(KeyCodeTest, QuadKeepsLeadingZeroAndLength)
{
  const QString text = "G:Galaxy 16 Bit Quad:()(0x0202300002120301)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getBitCount(), 16);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Number);
  const KeyCodeNumber number = result.getItem(1).value<KeyCodeNumber>();
  EXPECT_EQ(number.digits, "0202300002120301");
  EXPECT_EQ(number.digits.size(), 16);
}

TEST_F(KeyCodeTest, MixedWordsInArrays)
{
  const QString text =
      "G:X10 Dim 10 Bit:(0x193_SS_0x193_LS)(0x136_SS_0x136_LRS)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getBitCount(), 10);
  ASSERT_EQ(result.getItemType(0), KeyCode::ItemType::Array);
  ASSERT_EQ(result.getItemType(1), KeyCode::ItemType::Array);
  const QVariantList first = result.getItem(0).toList();
  ASSERT_EQ(first.size(), 4);
  expectNumber(first[0], 0, "193");
  expectText(first[1], "SS");
  expectNumber(first[2], 0, "193");
  expectText(first[3], "LS");
  const QVariantList second = result.getItem(1).toList();
  ASSERT_EQ(second.size(), 4);
  expectText(second[3], "LRS");
}

TEST_F(KeyCodeTest, NoBitCountInName)
{
  const QString text = "G:Philips RC6:()(0x1234)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getName(), "Philips RC6");
  EXPECT_EQ(result.getBitCount(), -1);
}

TEST_F(KeyCodeTest, OnlyTwoGroups)
{
  const QString text = "G:Rock IP:(Key)(Back):3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getName(), "Rock IP");
  EXPECT_EQ(result.getBitCount(), -1);
  EXPECT_EQ(result.getTrailer(), 3);
  expectText(result.getItem(0), "Key");
  expectText(result.getItem(1), "Back");
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, MalformedTokenIsKeptAsText)
{
  const QString text = "G:Tsh 32 Bit:(0x00FF48B7v)(Repeat)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  expectText(result.getItem(0), "0x00FF48B7v");
}

TEST_F(KeyCodeTest, TrailingWhitespaceIsIgnored)
{
  const QString text = "G:JVC 16 Bit:(Start)(0xC0C8)():3\t";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getTrailer(), 3);
  expectNumber(result.getItem(1), 0, "C0C8");
}

TEST_F(KeyCodeTest, SpaceBeforeTrailer)
{
  const QString text = "G:Tsh 32 Bit:(0x0AF5B24D)(Repeat)() :3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getTrailer(), 3);
  expectText(result.getItem(1), "Repeat");
  EXPECT_EQ(result.getItemType(2), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, MultiDigitTrailer)
{
  const QString text = "G:Test 8 Bit:()(0x12)():312";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getTrailer(), 312);
}

TEST_F(KeyCodeTest, ItemOutOfRange)
{
  const QString text = "G:SIRCS 15 Bit:()(0x5CE9)():3";
  KeyCode result;
  ASSERT_TRUE(KeyCode::parse(text, result));
  EXPECT_EQ(result.getItemType(-1), KeyCode::ItemType::Empty);
  EXPECT_EQ(result.getItemType(3), KeyCode::ItemType::Empty);
}

TEST_F(KeyCodeTest, InvalidInputFails)
{
  KeyCode result;
  EXPECT_FALSE(KeyCode::parse("", result));
  EXPECT_FALSE(KeyCode::parse("G", result));
  EXPECT_FALSE(KeyCode::parse("G:Sony 15 Bit", result));
  EXPECT_FALSE(KeyCode::parse("G:Sony 15 Bit:()(0x5CE9:3", result));
}

// ===========================================================================
// Line-separated fixture
// ===========================================================================

class KeyCodeFileTest: public ::testing::Test
{
  protected:
    const QString testPath = KEY_CODE_TEST_PATH;
};

TEST_F(KeyCodeFileTest, AllLinesParse)
{
  QFile file(testPath);
  ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text))<< "Could not open " << testPath.toStdString();
  QTextStream stream(&file);
  int lineNumber = 0;
  while (!stream.atEnd()) {
    ++lineNumber;
    const QString line = stream.readLine();
    if (line.trimmed().isEmpty()) {
      continue;
    }
    const QString text = line.trimmed();
    KeyCode result;
    ASSERT_TRUE(KeyCode::parse(text, result))<< "Parse failed at line " << lineNumber
    << ": " << text.toStdString();
    EXPECT_EQ(result.getGroup(), "G") << "Line " << lineNumber << ": "
        << text.toStdString();
    EXPECT_FALSE(result.getName().isEmpty()) << "Line " << lineNumber << ": "
        << text.toStdString();
  }
}
