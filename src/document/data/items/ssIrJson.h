// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <nlohmann/json.hpp>

#include "bin/ssIr/file.h"

namespace document
{
namespace data
{
namespace serialiser
{

using nlohmann::ordered_json;

//bin/ssIr/file.h
void toJson(ordered_json &out, const binary::ssIr::SerialStreamIr &stream);
void fromJson(const ordered_json &in, binary::ssIr::SerialStreamIr &stream);


}
}
}
