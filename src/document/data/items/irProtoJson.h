// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <nlohmann/json.hpp>

#include "bin/irProto/file.h"

namespace document
{
namespace data
{
namespace serialiser
{

using nlohmann::ordered_json;

//bin/irProto/file.h -- Item (std::pair<bool,uint16_t>)
void toJson(ordered_json &out, const binary::irProto::Item &item);
void fromJson(const ordered_json &in, binary::irProto::Item &item);

void toJson(ordered_json &out, const binary::irProto::TimingSectionIrHeader &header);
void fromJson(const ordered_json &in, binary::irProto::TimingSectionIrHeader &header);

void toJson(ordered_json &out, const binary::irProto::TimingSectionIrPayload &payload);
void fromJson(const ordered_json &in, binary::irProto::TimingSectionIrPayload &payload);

void toJson(ordered_json &out, const binary::irProto::TimingSection &section);
void fromJson(const ordered_json &in, binary::irProto::TimingSection &section);

void toJson(ordered_json &out, const binary::irProto::IrProto &proto, const std::string &name);
std::string fromJson(const ordered_json &in, binary::irProto::IrProto &proto);

}
}
}
