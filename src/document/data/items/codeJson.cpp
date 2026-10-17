// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QByteArray>

#include "codeJson.h"
#include "lib/qtHelpers.h"

using namespace std;

namespace document
{
namespace data
{
namespace serialiser
{

//---------------------------------------------------------------------------
// bin/timing.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const binary::Block &block)
{
  //segment_us is derived (mark_us + pause_us) -- don't store redundant data
  out["Mark"] = block.mark_us;
  out["Pause"] = block.pause_us;
}

void fromJson(const ordered_json &in, binary::Block &block)
{
  block = binary::Block::fromMarkPause(in.value("Mark", uint16_t(0)),
      in.value("Pause", uint16_t(0)));
}

void toJson(ordered_json &out, const binary::TimingStream &stream)
{
  ordered_json arr = ordered_json::array();
  for (const auto &block : stream.timings()) {
    ordered_json blockJson;
    toJson(blockJson, block);
    arr.push_back(blockJson);
  }
  out = arr;
}

void fromJson(const ordered_json &in, binary::TimingStream &stream)
{
  vector<uint16_t> markPause;

  if (in.is_array()) {
    for (const auto &blockJson : in) {
      binary::Block block = binary::Block::fromMarkPause(0, 0);
      fromJson(blockJson, block);
      markPause.push_back(block.mark_us);
      markPause.push_back(block.pause_us);
    }
  }

  stream = binary::TimingStream::fromMarkPause(markPause);
}

//---------------------------------------------------------------------------
// bin/irProto/code.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const binary::irProto::Group &group)
{
  out["Section"] = group.getSection();
  out["Bits"] = group.getBits();
  out["DataQuantumCount"] = group.getData().size();
  if (group.getBits() == 1) {
    //compact -> string
    auto data = binary::irProto::Code::listToU64bits(group.getData());
    out["Data"] = QString("0x" + QString::number(data, 16)).toStdString();
  } else {
    //array
    out["Data"] = group.getData();
  }
}

void fromJsonOld(const ordered_json &in, binary::irProto::Group &group,
    uint32_t idx)
{
  vector<uint8_t> data;

  auto dataIt = in.find("Data");
  if (dataIt != in.end()) {
    auto bitCount = in.value("DataBitCount", size_t(0));
    auto bits = qstr(dataIt->get<string>()).toULongLong(nullptr, 16);
    data = binary::irProto::Code::u64bitsToList(bitCount, bits);
  }
  group = binary::irProto::Group(idx, 1, data);
}

void fromJson(const ordered_json &in, binary::irProto::Group &group)
{
  vector<uint8_t> data;

  auto section = in.value("Section", 0);
  auto bits = in.value("Bits", 1);
  auto bitCount = in.value("DataQuantumCount", 0);
  auto dataIt = in.find("Data");
  if (dataIt != in.end()) {
    if (dataIt->is_string()) {
      auto hexValue = qstr(dataIt->get<string>()).toULongLong(nullptr, 16);
      data = binary::irProto::Code::u64bitsToList(bitCount, hexValue);
    } else if (dataIt->is_array()) {
      data = dataIt->get<vector<uint8_t>>();
    }
  }
  group = binary::irProto::Group(section, bits, data);
}

void toJson(ordered_json &out, const binary::irProto::Code &code)
{
  //index is part of parent
  out["Delay"] = code.getDelay();
  out["Repeats"] = code.getRepeats();

  ordered_json groupsArr = ordered_json::array();
  for (const auto &group : code.accessStartGroups()) {
    ordered_json groupJson;
    toJson(groupJson, group);
    groupsArr.push_back(groupJson);
  }
  out["StartGroups"] = groupsArr;

  groupsArr = ordered_json::array();
  for (const auto &group : code.accessRepeatGroups()) {
    ordered_json groupJson;
    toJson(groupJson, group);
    groupsArr.push_back(groupJson);
  }
  out["RepeatGroups"] = groupsArr;

  groupsArr = ordered_json::array();
  for (const auto &group : code.accessFinishGroups()) {
    ordered_json groupJson;
    toJson(groupJson, group);
    groupsArr.push_back(groupJson);
  }
  out["FinishGroups"] = groupsArr;
}

void fromJson(const ordered_json &in, binary::irProto::Code &code)
{
  vector<binary::irProto::Group> groups;

  code.setDelay(
      in.value("Delay", binary::irProto::Code::DEFAULT_PRE_SEND_SILENCE));

  if (in.contains("Control")) {
    //fixme pre v0.2 when to drop?
    code.setRepeats(
        in.value("DataFrameTxCount", binary::irProto::Code::DEFAULT_REPEATS));

    auto groupsIt = in.find("Sections");
    if (groupsIt != in.end()) {
      for (size_t i = 0; i < groupsIt->size(); i++) {
        binary::irProto::Group group;
        fromJsonOld((*groupsIt)[i], group, i);
        groups.push_back(group);
      }
    }

    auto control = in.value("Control", uint8_t(0));
    //0 = NEC (or generic + repeat) -> start + empty repeat (see createSimpleRepeatEmpty())
    //1 = Generic
    //3 = Philips RC-6
    //else -> n repeat groups
    switch (control) {
      case 0:
        code.accessStartGroups() = groups;
        code.setRepeatGroups( { { 1, { } } });
        break;
      default:
        code.accessRepeatGroups() = groups;
        break;
    }

  } else {
    code.setRepeats(
        in.value("Repeats", binary::irProto::Code::DEFAULT_REPEATS));

    auto parseGroups = [&in](const char *key,
        vector<binary::irProto::Group> &target) {
          target.clear();
          auto groupsIt = in.find(key);
          if (groupsIt != in.end()) {
            for (size_t i = 0; i < groupsIt->size(); i++) {
              binary::irProto::Group group;
              fromJson((*groupsIt)[i], group);
              target.push_back(group);
            }
          }
        };

    parseGroups("StartGroups", code.accessStartGroups());
    parseGroups("RepeatGroups", code.accessRepeatGroups());
    parseGroups("FinishGroups", code.accessFinishGroups());
  }
}

}
}
}
