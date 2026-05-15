// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::domain::Enum<T>
 */

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

#include "enum.h"

using namespace document::domain;


// ---------------------------------------------------------------------------
// Constructor: string -> enum value
// ---------------------------------------------------------------------------

TEST(EnumStringCtor, KnownValueMapsCorrectly)
{
    Enum<Locale> e("enu");
    EXPECT_EQ(e.getValue(), Locale::enu);
}

TEST(EnumStringCtor, CaseInsensitive)
{
    Enum<Locale> e("ENU");
    EXPECT_EQ(e.getValue(), Locale::enu);
}

TEST(EnumStringCtor, UnknownStringFallsBackToUnknown)
{
    Enum<Locale> e("garbage");
    EXPECT_EQ(e.getValue(), Locale::Unknown);
}

TEST(EnumStringCtor, UnknownStringPreservesSrc)
{
    Enum<Locale> e("garbage");
    EXPECT_EQ(e.getString(), "garbage");
}

// ---------------------------------------------------------------------------
// Constructor: enum value -> string
// ---------------------------------------------------------------------------

TEST(EnumValueCtor, KnownValue)
{
    Enum<Locale> e(Locale::deu);
    EXPECT_EQ(e.getValue(), Locale::deu);
    EXPECT_EQ(e.getString(), "deu");
}

TEST(EnumValueCtor, UnknownValue)
{
    Enum<Locale> e(Locale::Unknown);
    EXPECT_EQ(e.getValue(), Locale::Unknown);
}

// ---------------------------------------------------------------------------
// Static: isEnumValue
// ---------------------------------------------------------------------------

TEST(EnumIsEnumValue, KnownValue)
{
    EXPECT_TRUE(Enum<Locale>::isEnumValue("enu"));
}

TEST(EnumIsEnumValue, CaseInsensitive)
{
    EXPECT_TRUE(Enum<Locale>::isEnumValue("DEU"));
}

TEST(EnumIsEnumValue, UnknownReturnsFalse)
{
    // "Unknown" is explicitly excluded
    EXPECT_FALSE(Enum<Locale>::isEnumValue("Unknown"));
}

TEST(EnumIsEnumValue, GarbageReturnsFalse)
{
    EXPECT_FALSE(Enum<Locale>::isEnumValue("notavalue"));
}

// ---------------------------------------------------------------------------
// Static: getString(T v)
// ---------------------------------------------------------------------------

TEST(EnumGetStringStatic, KnownValue)
{
    EXPECT_EQ(Enum<Locale>::getString(Locale::enu), "enu");
}

// ---------------------------------------------------------------------------
// getStringList
// ---------------------------------------------------------------------------

TEST(EnumGetStringList, ContainsKnownValues)
{
    Enum<Locale> e(Locale::enu);
    auto list = e.getStringList();
    EXPECT_NE(std::find(list.begin(), list.end(), "enu"),  list.end());
    EXPECT_NE(std::find(list.begin(), list.end(), "deu"),  list.end());
}

TEST(EnumGetStringList, DoesNotContainUnknown)
{
    Enum<Locale> e(Locale::enu);
    auto list = e.getStringList();
    EXPECT_EQ(std::find(list.begin(), list.end(), "Unknown"), list.end());
}

TEST(EnumGetStringList, ContainsUnknownSrcWhenNotAValidValue)
{
    // src = "garbage" is not a valid enum value -> must be appended
    Enum<Locale> e("garbage");
    auto list = e.getStringList();
    EXPECT_NE(std::find(list.begin(), list.end(), "garbage"), list.end());
}

TEST(EnumGetStringList, DoesNotDuplicateValidSrc)
{
    // "enu" is already in the list via enum_for_each, should not appear twice
    Enum<Locale> e(Locale::enu);
    auto list = e.getStringList();
    int count = std::count(list.begin(), list.end(), "enu");
    EXPECT_EQ(count, 1);
}
