// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <nlohmann/json.hpp>

#include "device.h"

namespace document
{
namespace data
{
namespace serialiser
{

using nlohmann::ordered_json;

//button.h
std::string buttonTypeToString(item::ButtonType t);
item::ButtonType buttonTypeFromString(const std::string &s);

void toJson(ordered_json &out, const item::Button &button);
void fromJson(const ordered_json &in, item::Button &button);

//action.h
void toJson(ordered_json &out, const item::SequenceItem &seqItem);
void fromJson(const ordered_json &in, item::SequenceItem &seqItem);

void toJson(ordered_json &out, const item::DeviceAction &action);
void fromJson(const ordered_json &in, item::DeviceAction &action);

//state.h
void toJson(ordered_json &out, const item::DiscreteActions &actions);
void fromJson(const ordered_json &in, item::DiscreteActions &actions);

void toJson(ordered_json &out, const item::RelativeActions &actions);
void fromJson(const ordered_json &in, item::RelativeActions &actions);

void toJson(ordered_json &out, const item::StateMachine &sm);
void fromJson(const ordered_json &in, item::StateMachine &sm);

//digits.h
void toJson(ordered_json &out, const item::Digits &digits);
void fromJson(const ordered_json &in, item::Digits &digits);

void toJson(ordered_json &out, const item::Numpad &numpad);
void fromJson(const ordered_json &in, item::Numpad &numpad);

//commands.h
void toJson(ordered_json &out, const item::RawCommand &cmd);
void fromJson(const ordered_json &in, item::RawCommand &cmd);

void toJson(ordered_json &out, const item::ProtoCommand &cmd);
void fromJson(const ordered_json &in, item::ProtoCommand &cmd);

void toJson(ordered_json &out, const item::Commands &commands);
void fromJson(const ordered_json &in, item::Commands &commands);

//device.h
void toJson(ordered_json &out, const item::Device &device);
void fromJson(const ordered_json &in, item::Device &device);

}
}
}
