// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <nlohmann/json.hpp>

#include "blob.h"
#include "bin/timing.h"
#include "bin/irProto/code.h"

namespace document
{
namespace data
{
namespace serialiser
{

using nlohmann::ordered_json;

//bin/timing.h
void toJson(ordered_json &out, const binary::Block &block);
void fromJson(const ordered_json &in, binary::Block &block);

void toJson(ordered_json &out, const binary::TimingStream &stream);
void fromJson(const ordered_json &in, binary::TimingStream &stream);

//bin/irProto/code.h
void toJson(ordered_json &out, const binary::irProto::Section &section);
void fromJson(const ordered_json &in, binary::irProto::Section &section);

void toJson(ordered_json &out, const binary::irProto::Code &code);
void fromJson(const ordered_json &in, binary::irProto::Code &code);

}
}
}
