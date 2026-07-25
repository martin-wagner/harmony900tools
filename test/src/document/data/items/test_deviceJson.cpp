// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for document::data::serial (de)serialization of item::Device
 * and its dependencies (Button, DeviceAction, StateMachine, Numpad, Commands).
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "deviceJson.h"

using namespace document::data;
using namespace document::data::item;

// ---------------------------------------------------------------------------
// Property<T>: Used gating contract
// ---------------------------------------------------------------------------

TEST(DevicePropertyUsed, UnusedPropertyIsOmittedFromJson)
{
    Device d(1);
    d.audioSwitch.setIncluded(Used::NO);

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    EXPECT_FALSE(j.contains("AudioSwitch"));
}

TEST(DevicePropertyUsed, UsedPropertyIsWrittenToJson)
{
    Device d(1);
    d.audioSwitch.set(true);
    d.audioSwitch.setIncluded(Used::YES);

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    ASSERT_TRUE(j.contains("AudioSwitch"));
    EXPECT_EQ(j["AudioSwitch"], true);
}

TEST(DevicePropertyUsed, MissingKeyOnReadSetsUsedToNo)
{
    nlohmann::ordered_json j;
    j["Id"] = 1;
    // audioSwitch intentionally absent

    Device d(1);
    d.audioSwitch.setIncluded(Used::YES); // start as YES to prove it flips
    serialiser::fromJson(j, d);

    EXPECT_EQ(d.audioSwitch.isIncluded(), Used::NO);
}

TEST(DevicePropertyUsed, PresentKeyOnReadSetsUsedToYes)
{
    nlohmann::ordered_json j;
    j["Id"] = 1;
    j["AudioSwitch"] = true;

    Device d(1);
    d.audioSwitch.setIncluded(Used::NO); // start as NO to prove it flips
    serialiser::fromJson(j, d);

    EXPECT_EQ(d.audioSwitch.isIncluded(), Used::YES);
    EXPECT_EQ(d.audioSwitch.get(), true);
}

// ---------------------------------------------------------------------------
// PropertyEnum<T>: written/read as string
// ---------------------------------------------------------------------------

TEST(DevicePropertyEnum, WrittenAsString)
{
    Device d(1);
    d.type.set(Enum<DeviceType>(DeviceType::Television));

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    EXPECT_EQ(j["Type"], "Television");
}

TEST(DevicePropertyEnum, ReadFromString)
{
    nlohmann::ordered_json j;
    j["Id"] = 1;
    j["Type"] = "Amplifier";

    Device d(1);
    serialiser::fromJson(j, d);

    EXPECT_EQ(d.type.get().getValue(), DeviceType::Amplifier);
}

// ---------------------------------------------------------------------------
// Button / ButtonType
// ---------------------------------------------------------------------------

TEST(ButtonJson, RoundTripHardButton)
{
    Button b(ButtonType::Hard);
    b.action.set("Power");

    nlohmann::ordered_json j;
    serialiser::toJson(j, b);
    EXPECT_EQ(j["Action"], "Power");

    Button b2(ButtonType::Hard);
    serialiser::fromJson(j, b2);
    EXPECT_EQ(b2.action.get(), "Power");
}

TEST(ButtonJson, RoundTripSoftButton)
{
    Button b(ButtonType::Soft);
    b.action.set("Power");
    b.name.set("Test").setIncluded(Used::YES);

    nlohmann::ordered_json j;
    serialiser::toJson(j, b);
    EXPECT_EQ(j["Action"], "Power");
    EXPECT_EQ(j["Name"], "Test");

    Button b2(ButtonType::Hard);
    serialiser::fromJson(j, b2);
    EXPECT_EQ(b2.action.get(), "Power");
    EXPECT_EQ(b2.name.get(), "Test");
}

TEST(ButtonJson, RoundTripSoftButtonReconstructsTypeFromDevice)
{
    Device d(1);
    Button soft(ButtonType::Soft);
    soft.name.set("Guide");
    soft.name.setIncluded(Used::YES);
    d.getButtons().push_back(soft);

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    Device d2(1);
    serialiser::fromJson(j, d2);

    ASSERT_EQ(d2.getButtons().size(), 1u);
    EXPECT_EQ(d2.getButtons()[0].getButtonType(), ButtonType::Soft);
    EXPECT_EQ(d2.getButtons()[0].name.get(), "Guide");
}

// ---------------------------------------------------------------------------
// DeviceAction / SequenceItem
// ---------------------------------------------------------------------------

TEST(DeviceActionJson, SequenceRoundTrips)
{
    DeviceAction action;
    action.actionType.set(Enum<ActionType>(ActionType::SetAction));
    action.actionType.setIncluded(Used::YES);

    SequenceItem step;
    step.opcode.set(Enum<Operation>(Operation::SendCommand));
    step.opcode.setIncluded(Used::YES);
    step.cmd.set("Volume+");
    step.cmd.setIncluded(Used::YES);
    action.sequence.push_back(step);

    nlohmann::ordered_json j;
    serialiser::toJson(j, action);

    DeviceAction action2;
    serialiser::fromJson(j, action2);

    ASSERT_EQ(action2.sequence.size(), 1u);
    EXPECT_EQ(action2.sequence[0].opcode.get().getValue(), Operation::SendCommand);
    EXPECT_EQ(action2.sequence[0].cmd.get(), "Volume+");
}

TEST(DeviceActionJson, EmptySequenceOmitsKey)
{
    DeviceAction action;

    nlohmann::ordered_json j;
    serialiser::toJson(j, action);

    EXPECT_FALSE(j.contains("sequence"));
}

// ---------------------------------------------------------------------------
// StateMachine / DiscreteActions / RelativeActions
// ---------------------------------------------------------------------------

TEST(StateMachineJson, DiscreteStatesRoundTrip)
{
    StateMachine sm;
    sm.discrete.states = {"On", "Off"};

    nlohmann::ordered_json j;
    serialiser::toJson(j, sm);

    StateMachine sm2;
    serialiser::fromJson(j, sm2);

    EXPECT_EQ(sm2.discrete.states, sm.discrete.states);
}

TEST(StateMachineJson, OptionalStartActionAbsentByDefault)
{
    StateMachine sm;

    nlohmann::ordered_json j;
    serialiser::toJson(j, sm);

    EXPECT_FALSE(j.contains("StartAction"));
}

TEST(StateMachineJson, OptionalStartActionRoundTripsWhenPresent)
{
    StateMachine sm;
    DeviceAction start;
    start.actionType.set(Enum<ActionType>(ActionType::StartAction));
    start.actionType.setIncluded(Used::YES);
    sm.startAction = start;

    nlohmann::ordered_json j;
    serialiser::toJson(j, sm);
    ASSERT_TRUE(j.contains("StartAction"));

    StateMachine sm2;
    serialiser::fromJson(j, sm2);

    ASSERT_TRUE(sm2.startAction.has_value());
    EXPECT_EQ(sm2.startAction->actionType.get().getValue(), ActionType::StartAction);
}

// ---------------------------------------------------------------------------
// Numpad / Digits (fixed-size array)
// ---------------------------------------------------------------------------

TEST(NumpadJson, DigitsArrayPreservesIndexPosition)
{
    Numpad numpad;
    Digits digits{};
    digits[3].actionType.set(Enum<ActionType>(ActionType::SetAction));
    digits[3].actionType.setIncluded(Used::YES);
    numpad.first = digits;

    nlohmann::ordered_json j;
    serialiser::toJson(j, numpad);

    Numpad numpad2;
    serialiser::fromJson(j, numpad2);

    ASSERT_TRUE(numpad2.first.has_value());
    EXPECT_EQ(numpad2.first.value()[3].actionType.get().getValue(), ActionType::SetAction);
    // untouched slots keep their (unused) default
    EXPECT_EQ(numpad2.first.value()[0].actionType.isIncluded(), Used::NO);
}

TEST(NumpadJson, AbsentOptionalDigitsStayNullopt)
{
    Numpad numpad;

    nlohmann::ordered_json j;
    serialiser::toJson(j, numpad);

    Numpad numpad2;
    serialiser::fromJson(j, numpad2);

    EXPECT_FALSE(numpad2.first.has_value());
    EXPECT_FALSE(numpad2.middle.has_value());
    EXPECT_FALSE(numpad2.last.has_value());
}

// ---------------------------------------------------------------------------
// Commands / RawCommand / ProtoCommand (binary data as base64)
// ---------------------------------------------------------------------------

TEST(ProtoCommandJson, BinaryDataRoundTripsViaBase64)
{
    ProtoCommand proto;
    proto.data.set({0x00, 0x01, 0xFF, 0x7E, 0x80});
    proto.name.set("ProtoTest");

    nlohmann::ordered_json j;
    serialiser::toJson(j, proto);

    ASSERT_TRUE(j["Data"].is_string());

    ProtoCommand proto2;
    serialiser::fromJson(j, proto2);

    EXPECT_EQ(proto2.data.get(), proto.data.get());
    EXPECT_EQ(proto2.data.isIncluded(), Used::YES);
}

TEST(ProtoCommandJson, UnusedBinaryDataOmitsKey)
{
    ProtoCommand proto;
    // proto.data defaults to Used::YES per commands.h -- explicitly disable for this test
    proto.data.setIncluded(Used::NO);

    nlohmann::ordered_json j;
    serialiser::toJson(j, proto);

    EXPECT_FALSE(j.contains("Data"));
}

TEST(CommandsJson, RawAndProtoListsRoundTrip)
{
    Commands commands;

    RawCommand raw;
    raw.name.set("RawOne");
    raw.streamIndex.set(3);
    commands.getRawCommands().push_back(raw);

    ProtoCommand proto;
    proto.name.set("ProtoOne");
    proto.data.set({0x10, 0x20});
    commands.getProtoCommands().push_back(proto);

    nlohmann::ordered_json j;
    serialiser::toJson(j, commands);

    Commands commands2;
    serialiser::fromJson(j, commands2);

    ASSERT_EQ(commands2.getRawCommands().size(), 1u);
    EXPECT_EQ(commands2.getRawCommands()[0].name.get(), "RawOne");

    ASSERT_EQ(commands2.getProtoCommands().size(), 1u);
    EXPECT_EQ(commands2.getProtoCommands()[0].data.get(), proto.data.get());
}

// ---------------------------------------------------------------------------
// UnknownElement passthrough
// ---------------------------------------------------------------------------

TEST(DeviceUnknownProperties, RoundTripsNestedChildren)
{
    Device d(1);

    UnknownElement child("Child", {{"attr", "1"}}, "childText");
    UnknownElement parent("Parent", {{"attr", "0"}}, "parentText", {child});
    d.getUnknownProperties().push_back(parent);

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    Device d2(1);
    serialiser::fromJson(j, d2);

    ASSERT_EQ(d2.getUnknownProperties().size(), 1u);
    const auto &p = d2.getUnknownProperties()[0];
    EXPECT_EQ(p.tag, "Parent");
    EXPECT_EQ(p.text, "parentText");
    ASSERT_EQ(p.children.size(), 1u);
    EXPECT_EQ(p.children[0].tag, "Child");
}

TEST(DeviceUnknownProperties, EmptyListOmitsKey)
{
    Device d(1);

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    EXPECT_FALSE(j.contains("unknownProperties"));
}

// ---------------------------------------------------------------------------
// Full Device round-trip
// ---------------------------------------------------------------------------

TEST(DeviceJson, FullRoundTripProducesIdenticalJson)
{
    Device d(42);
    d.mnf.set("TestMnf");
    d.mnf.setIncluded(Used::YES);
    d.type.set(Enum<DeviceType>(DeviceType::Television));

    Button b(ButtonType::Hard);
    b.action.set("Power");
    d.getButtons().push_back(b);

    StateMachine sm;
    sm.smType.set(Enum<StateMachineDeviceType>(StateMachineDeviceType::Power));
    d.getStateMachines().push_back(sm);

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    Device d2(42);
    serialiser::fromJson(j, d2);

    nlohmann::ordered_json j2;
    serialiser::toJson(j2, d2);

    EXPECT_EQ(j, j2);
}

TEST(DeviceJson, IdIsAlwaysWritten)
{
    Device d(77);

    nlohmann::ordered_json j;
    serialiser::toJson(j, d);

    ASSERT_TRUE(j.contains("Id"));
    EXPECT_EQ(j["Id"], 77);
}
