// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <nlohmann/json.hpp>

#include "userInfo.h"
#include "controllerInfo.h"
#include "blob.h"

namespace document
{
namespace data
{
namespace serialiser
{

using nlohmann::ordered_json;

//userInfo.h
void toJson(ordered_json &out, const item::UserInfo &user);
void fromJson(const ordered_json &in, item::UserInfo &user);

//controllerInfo.h
void toJson(ordered_json &out, const item::ControllerInfo &controller);
void fromJson(const ordered_json &in, item::ControllerInfo &controller);

//items/blob.h
void toJson(ordered_json &out, const item::Blob &blob);
item::Blob fromJson(const ordered_json &in);

}
}
}
