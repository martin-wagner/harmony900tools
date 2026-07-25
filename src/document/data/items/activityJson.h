// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <nlohmann/json.hpp>

#include "activity.h"

namespace document
{
namespace data
{
namespace serialiser
{

using nlohmann::ordered_json;

//button.h (Channel)
void toJson(ordered_json &out, const item::Channel &channel);
void fromJson(const ordered_json &in, item::Channel &channel);

//role.h
void toJson(ordered_json &out, const item::Role &role);
void fromJson(const ordered_json &in, item::Role &role);

//activity.h
void toJson(ordered_json &out, const item::Activity &activity);
void fromJson(const ordered_json &in, item::Activity &activity);

}
}
}
