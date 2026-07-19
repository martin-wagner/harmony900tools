// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::data::serialiser (de)serialization of item::UserInfo
 * and item::ControllerInfo.
 */

#include <gtest/gtest.h>
#include <string>

#include "commonJson.h"

using namespace document::data;
using namespace document::data::item;

// ---------------------------------------------------------------------------
// Blob
// ---------------------------------------------------------------------------

TEST(BlobJson, RoundTripPreservesFileNameAndPermissions)
{
  item::Blob blob("icon.png", {0x01, 0x02, 0x03}, {7, 4, 4});

  serialiser::ordered_json j;
  serialiser::toJson(j, blob);

  EXPECT_EQ(j["File"], "icon.png");

  auto blob2 = serialiser::fromJson(j);

  EXPECT_EQ(blob2.getFile(), "icon.png");
  EXPECT_EQ(blob2.getPermissions(), (item::Permissions{7, 4, 4}));
}

TEST(BlobJson, BinaryDataRoundTripsViaBase64)
{
  std::vector<uint8_t> data = {0x00, 0xFF, 0x7E, 0x80, 0x10};
  item::Blob blob("file.bin", data);

  serialiser::ordered_json j;
  serialiser::toJson(j, blob);

  ASSERT_TRUE(j["Data"].is_string());

  auto blob2 = serialiser::fromJson(j);

  EXPECT_EQ(blob2.getData(), data);
}

// ---------------------------------------------------------------------------
// UserInfo: id handling
// ---------------------------------------------------------------------------

TEST(UserInfoJson, IdIsAlwaysWritten)
{
    UserInfo user;
    user.setId(1234);

    nlohmann::ordered_json j;
    serialiser::toJson(j, user);

    ASSERT_TRUE(j.contains("Id"));
    EXPECT_EQ(j["Id"], 1234);
}

TEST(UserInfoJson, MissingIdOnReadFallsBackToDefault)
{
    nlohmann::ordered_json j; // no "Id" key

    UserInfo user;
    user.setId(999); // start with a non-default value to prove the fallback applies
    serialiser::fromJson(j, user);

    EXPECT_EQ(user.getId(), static_cast<uint32_t>(UserInfo::DEFAULT_USERID));
}

// ---------------------------------------------------------------------------
// UserInfo: Used gating contract
// ---------------------------------------------------------------------------

TEST(UserInfoJson, UnusedPropertyOmitted)
{
    UserInfo user;
    user.newDeviceFound.setIncluded(Used::NO); // matches header default

    nlohmann::ordered_json j;
    serialiser::toJson(j, user);

    EXPECT_FALSE(j.contains("NewDeviceFound"));
}

TEST(UserInfoJson, UsedPropertyWritten)
{
    UserInfo user;
    user.firstName.set("Ada");
    user.firstName.setIncluded(Used::YES);

    nlohmann::ordered_json j;
    serialiser::toJson(j, user);

    ASSERT_TRUE(j.contains("FirstName"));
    EXPECT_EQ(j["FirstName"], "Ada");
}

// ---------------------------------------------------------------------------
// UserInfo: enum properties (Locale, TimeFormat)
// ---------------------------------------------------------------------------

TEST(UserInfoJson, LocaleWrittenAsString)
{
    UserInfo user;
    user.locale.set(Enum<Locale>(Locale::deu));

    nlohmann::ordered_json j;
    serialiser::toJson(j, user);

    EXPECT_EQ(j["Locale"], "deu");
}

TEST(UserInfoJson, LocaleReadFromString)
{
    nlohmann::ordered_json j;
    j["Locale"] = "fra";

    UserInfo user;
    serialiser::fromJson(j, user);

    EXPECT_EQ(user.locale.get().getValue(), Locale::fra);
}

// ---------------------------------------------------------------------------
// UserInfo: full round trip
// ---------------------------------------------------------------------------

TEST(UserInfoJson, FullRoundTripProducesIdenticalJson)
{
    UserInfo user;
    user.setId(1234);
    user.firstName.set("Ada");
    user.firstName.setIncluded(Used::YES);
    user.locale.set(Enum<Locale>(Locale::deu));

    nlohmann::ordered_json j;
    serialiser::toJson(j, user);

    UserInfo user2;
    serialiser::fromJson(j, user2);

    nlohmann::ordered_json j2;
    serialiser::toJson(j2, user2);

    EXPECT_EQ(j, j2);
    EXPECT_EQ(user2.getId(), 1234u);
}

// ---------------------------------------------------------------------------
// ControllerInfo: id handling
// ---------------------------------------------------------------------------

TEST(ControllerInfoJson, IdIsAlwaysWritten)
{
    ControllerInfo controller;
    controller.setId(7);

    nlohmann::ordered_json j;
    serialiser::toJson(j, controller);

    ASSERT_TRUE(j.contains("Id"));
    EXPECT_EQ(j["Id"], 7);
}

TEST(ControllerInfoJson, MissingIdOnReadFallsBackToDefault)
{
    nlohmann::ordered_json j; // no "Id" key

    ControllerInfo controller;
    controller.setId(42);
    serialiser::fromJson(j, controller);

    EXPECT_EQ(controller.getId(), static_cast<uint32_t>(ControllerInfo::DEFAULT_ID));
}

// ---------------------------------------------------------------------------
// ControllerInfo: full round trip
// ---------------------------------------------------------------------------

TEST(ControllerInfoJson, FullRoundTripProducesIdenticalJson)
{
    ControllerInfo controller;
    controller.setId(7);
    controller.mnf.set("Acme");
    controller.mnf.setIncluded(Used::YES);

    nlohmann::ordered_json j;
    serialiser::toJson(j, controller);

    ControllerInfo controller2;
    serialiser::fromJson(j, controller2);

    nlohmann::ordered_json j2;
    serialiser::toJson(j2, controller2);

    EXPECT_EQ(j, j2);
    EXPECT_EQ(controller2.getId(), 7u);
}

TEST(ControllerInfoJson, UnknownPropertiesRoundTrip)
{
    ControllerInfo controller;
    controller.getUnknownProperties().push_back(UnknownElement("Extra", {}, "value"));

    nlohmann::ordered_json j;
    serialiser::toJson(j, controller);

    ControllerInfo controller2;
    serialiser::fromJson(j, controller2);

    ASSERT_EQ(controller2.getUnknownProperties().size(), 1u);
    EXPECT_EQ(controller2.getUnknownProperties()[0].tag, "Extra");
}
