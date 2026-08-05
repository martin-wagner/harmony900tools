// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::data::serial (de)serialization of item::Activity
 * and its dependencies (Channel, Role).
 */

#include <gtest/gtest.h>
#include <string>

#include "activityJson.h"

using namespace document::data;
using namespace document::data::item;

// ---------------------------------------------------------------------------
// Activity: id handling (constructor-only, no setter)
// ---------------------------------------------------------------------------

TEST(ActivityJson, IdIsAlwaysWritten)
{
    Activity activity(99);

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    ASSERT_TRUE(j.contains("Id"));
    EXPECT_EQ(j["Id"], 99);
}

TEST(ActivityJson, ReadDoesNotChangeExistingId)
{
    // Activity has no setId(); id must be supplied at construction. fromJson
    // must not attempt to alter it even if "Id" is present in the JSON.
    nlohmann::ordered_json j;
    j["Id"] = 5; // different from the id the object was constructed with

    Activity activity(99);
    serialiser::fromJson(j, activity);

    EXPECT_EQ(activity.getId(), 99u);
}

// ---------------------------------------------------------------------------
// Activity: Used gating contract
// ---------------------------------------------------------------------------

TEST(ActivityJson, UnusedPropertyOmitted)
{
    Activity activity(1);
    activity.hideModeControl.setIncluded(Used::NO); // matches header default

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    EXPECT_FALSE(j.contains("HideModeControl"));
}

TEST(ActivityJson, UsedPropertyWritten)
{
    Activity activity(1);
    activity.label.set("Watch TV");

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    ASSERT_TRUE(j.contains("Label"));
    EXPECT_EQ(j["Label"], "Watch TV");
}

// ---------------------------------------------------------------------------
// Activity: enum properties
// ---------------------------------------------------------------------------

TEST(ActivityJson, TypeWrittenAsString)
{
    Activity activity(1);
    activity.type.set(Enum<ActivityType>(ActivityType::VirtualDvd));

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    EXPECT_EQ(j["Type"], "VirtualDvd");
}

// ---------------------------------------------------------------------------
// Channel
// ---------------------------------------------------------------------------

TEST(ChannelJson, RoundTrips)
{
    Channel ch;
    ch.station.set("HD1");
    ch.channel.set(5);

    nlohmann::ordered_json j;
    serialiser::toJson(j, ch);

    Channel ch2;
    serialiser::fromJson(j, ch2);

    EXPECT_EQ(ch2.station.get(), "HD1");
    EXPECT_EQ(ch2.channel.get(), 5u);
}

TEST(ActivityJson, ChannelListRoundTrips)
{
    Activity activity(1);
    Channel ch;
    ch.station.set("HD1");
    ch.channel.set(5);
    activity.getChannels().push_back(ch);

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    Activity activity2(1);
    serialiser::fromJson(j, activity2);

    ASSERT_EQ(activity2.getChannels().size(), 1u);
    EXPECT_EQ(activity2.getChannels()[0].station.get(), "HD1");
}

// ---------------------------------------------------------------------------
// Role -- plain Enum<T> member (no Used gating, always written)
// ---------------------------------------------------------------------------

TEST(RoleJson, RoleEnumIsAlwaysWrittenRegardlessOfUsed)
{
    // Role::role is a bare Enum<T>, not Property-wrapped -- it has no
    // Used concept and must always be serialized.
    Role role;
    role.deviceId.set(3);
    role.role = Enum<DeviceRole>(DeviceRole::DISPLAY);

    nlohmann::ordered_json j;
    serialiser::toJson(j, role);

    ASSERT_TRUE(j.contains("Role"));
    EXPECT_EQ(j["Role"], "DISPLAY");
}

TEST(RoleJson, RoundTrips)
{
    Role role;
    role.deviceId.set(3);
    role.role = Enum<DeviceRole>(DeviceRole::VOLUME);

    nlohmann::ordered_json j;
    serialiser::toJson(j, role);

    Role role2;
    serialiser::fromJson(j, role2);

    EXPECT_EQ(role2.deviceId.get(), 3u);
    EXPECT_EQ(role2.role.getValue(), DeviceRole::VOLUME);
}

TEST(ActivityJson, RoleListRoundTrips)
{
    Activity activity(1);
    Role role;
    role.deviceId.set(3);
    role.role = Enum<DeviceRole>(DeviceRole::DISPLAY);
    activity.getRoles().push_back(role);

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    Activity activity2(1);
    serialiser::fromJson(j, activity2);

    ASSERT_EQ(activity2.getRoles().size(), 1u);
    EXPECT_EQ(activity2.getRoles()[0].role.getValue(), DeviceRole::DISPLAY);
}

// ---------------------------------------------------------------------------
// Activity: buttons reuse Button/ButtonType serialization from deviceJson
// ---------------------------------------------------------------------------

TEST(ActivityJson, ButtonListPreservesButtonType)
{
    Activity activity(1);
    Button softButton;
    softButton.action.set("Mute");
    activity.getSoftButtons().push_back(softButton);

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    Activity activity2(1);
    serialiser::fromJson(j, activity2);

    ASSERT_EQ(activity2.getSoftButtons().size(), 1u);
    EXPECT_EQ(activity2.getSoftButtons()[0].action.get(), "Mute");
}

// ---------------------------------------------------------------------------
// Activity: plain uint32_t id lists (power on/off devices)
// ---------------------------------------------------------------------------

TEST(ActivityJson, PowerOnOffDeviceIdsRoundTrip)
{
    Activity activity(1);
    activity.getPowerOnDevices().push_back(1);
    activity.getPowerOnDevices().push_back(2);
    activity.getPowerOffDevices().push_back(3);

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    Activity activity2(1);
    serialiser::fromJson(j, activity2);

    EXPECT_EQ(activity2.getPowerOnDevices(), (std::vector<uint32_t>{1, 2}));
    EXPECT_EQ(activity2.getPowerOffDevices(), (std::vector<uint32_t>{3}));
}

TEST(ActivityJson, EmptyPowerDeviceListsOmitKeys)
{
    Activity activity(1);

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    EXPECT_FALSE(j.contains("powerOnDeviceIds"));
    EXPECT_FALSE(j.contains("powerOffDeviceIds"));
}

// ---------------------------------------------------------------------------
// Activity: full round trip
// ---------------------------------------------------------------------------

TEST(ActivityJson, FullRoundTripProducesIdenticalJson)
{
    Activity activity(99);
    activity.label.set("Watch TV");
    activity.type.set(Enum<ActivityType>(ActivityType::VirtualGeneric));

    Channel ch;
    ch.station.set("HD1");
    ch.channel.set(5);
    activity.getChannels().push_back(ch);

    Role role;
    role.deviceId.set(3);
    role.role = Enum<DeviceRole>(DeviceRole::DISPLAY);
    activity.getRoles().push_back(role);

    Button ab;
    ab.action.set("Mute");
    activity.getSoftButtons().push_back(ab);

    activity.getPowerOnDevices().push_back(1);
    activity.getPowerOnDevices().push_back(2);

    nlohmann::ordered_json j;
    serialiser::toJson(j, activity);

    Activity activity2(99);
    serialiser::fromJson(j, activity2);

    nlohmann::ordered_json j2;
    serialiser::toJson(j2, activity2);

    EXPECT_EQ(j, j2);
}
