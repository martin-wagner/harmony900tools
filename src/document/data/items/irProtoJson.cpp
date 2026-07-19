// SPDX-License-Identifier: LGPL-2.1-or-later

#include "irProtoJson.h"

using namespace std;

namespace document
{
namespace data
{
namespace serialiser
{

//---------------------------------------------------------------------------
// bin/irProto/file.h -- Item (std::pair<bool,uint16_t>)
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const binary::irProto::Item &item)
{
  out["State"] = item.first;
  out["Time"] = item.second;
}

void fromJson(const ordered_json &in, binary::irProto::Item &item)
{
  item.first = in.value("State", false);
  item.second = in.value("Time", uint16_t(0));
}

namespace
{

ordered_json toJsonItemList(const std::vector<binary::irProto::Item> &items)
{
  ordered_json arr = ordered_json::array();
  for (const auto &item : items) {
    ordered_json itemJson;
    toJson(itemJson, item);
    arr.push_back(itemJson);
  }
  return arr;
}

std::vector<binary::irProto::Item> fromJsonItemList(const ordered_json &in)
{
  std::vector<binary::irProto::Item> items;
  if (in.is_array()) {
    for (const auto &itemJson : in) {
      binary::irProto::Item item;
      fromJson(itemJson, item);
      items.push_back(item);
    }
  }
  return items;
}

}

void toJson(ordered_json &out,
    const binary::irProto::TimingSectionIrHeader &header)
{
  out = toJsonItemList(header.accessStream());
}

void fromJson(const ordered_json &in,
    binary::irProto::TimingSectionIrHeader &header)
{
  header = binary::irProto::TimingSectionIrHeader();
  for (const auto &item : fromJsonItemList(in)) {
    header.addItem(item);
  }
}

//---------------------------------------------------------------------------
// TimingSectionIrPayload
//---------------------------------------------------------------------------

void toJson(ordered_json &out,
    const binary::irProto::TimingSectionIrPayload &payload)
{
  out["FalsePair"] = toJsonItemList(payload.accessFalsePair());
  out["TruePair"] = toJsonItemList(payload.accessTruePair());
}

void fromJson(const ordered_json &in,
    binary::irProto::TimingSectionIrPayload &payload)
{
  payload = binary::irProto::TimingSectionIrPayload();

  auto falseIt = in.find("FalsePair");
  if (falseIt != in.end()) {
    payload.setFalsePair(fromJsonItemList(*falseIt));
  }

  auto trueIt = in.find("TruePair");
  if (trueIt != in.end()) {
    payload.setTruePair(fromJsonItemList(*trueIt));
  }
}

//---------------------------------------------------------------------------
// TimingSection
//---------------------------------------------------------------------------

namespace
{

std::string ctrl0ToString(binary::irProto::TimingSection::Ctrl0 c)
{
  switch (c) {
    case binary::irProto::TimingSection::Ctrl0::IS_REPEAT_FRAME:
      return "IsRepeatFrame";
    case binary::irProto::TimingSection::Ctrl0::IS_DATA_FRAME:
      return "IsDataFrame";
  }
  return "IsDataFrame";
}

binary::irProto::TimingSection::Ctrl0 ctrl0FromString(const std::string &s)
{
  if (s == "IsRepeatFrame") {
    return binary::irProto::TimingSection::Ctrl0::IS_REPEAT_FRAME;
  }
  return binary::irProto::TimingSection::Ctrl0::IS_DATA_FRAME;
}

std::string ctrl1ToString(binary::irProto::TimingSection::Ctrl1 c)
{
  switch (c) {
    case binary::irProto::TimingSection::Ctrl1::NOT_DATA_FRAME:
      return "NotDataFrame"; //== NoPayload, same value 0
    case binary::irProto::TimingSection::Ctrl1::DATA_ONE_PAIR:
      return "DataOnePair";
    case binary::irProto::TimingSection::Ctrl1::DATA_TWO_PAIRS:
      return "DataTwoPairs";
  }
  return "NotDataFrame";
}

binary::irProto::TimingSection::Ctrl1 ctrl1FromString(const std::string &s)
{
  if (s == "DataOnePair") {
    return binary::irProto::TimingSection::Ctrl1::DATA_ONE_PAIR;
  }
  if (s == "DataTwoPairs") {
    return binary::irProto::TimingSection::Ctrl1::DATA_TWO_PAIRS;
  }
  return binary::irProto::TimingSection::Ctrl1::NOT_DATA_FRAME;
}

}

void toJson(ordered_json &out, const binary::irProto::TimingSection &section)
{
  out["BitCount"] = section.getBitCount();
  out["Toggle"] = section.getToggle();
  out["Timing"] = section.getTiming();
  out["Ctrl0"] = ctrl0ToString(section.getCtrl0());
  out["Ctrl1"] = ctrl1ToString(section.getCtrl1());

  ordered_json sofJson;
  toJson(sofJson, section.getSoF());
  out["SoF"] = sofJson;

  ordered_json dataJson;
  toJson(dataJson, section.getData());
  out["Data"] = dataJson;

  ordered_json eofJson;
  toJson(eofJson, section.getEoF());
  out["EoF"] = eofJson;
}

void fromJson(const ordered_json &in, binary::irProto::TimingSection &section)
{
  section.setBitCount(in.value("BitCount", uint16_t(0)));
  section.setToggle(
      in.value("Toggle",
          uint16_t(binary::irProto::TimingSection::TOGGLE_NONE)));
  section.setTiming(
      in.value("Timing",
          uint32_t(binary::irProto::TimingSection::PAUSE_IN_EOF)));
  section.setCtrl0(
      ctrl0FromString(in.value("Ctrl0", std::string("IsDataFrame"))));
  section.setCtrl1(
      ctrl1FromString(in.value("Ctrl1", std::string("DataTwoPairs"))));

  auto sofIt = in.find("SoF");
  if (sofIt != in.end()) {
    binary::irProto::TimingSectionIrHeader sof;
    fromJson(*sofIt, sof);
    section.setSoF(sof);
  }

  auto dataIt = in.find("Data");
  if (dataIt != in.end()) {
    binary::irProto::TimingSectionIrPayload data;
    fromJson(*dataIt, data);
    section.setData(data);
  }

  auto eofIt = in.find("EoF");
  if (eofIt != in.end()) {
    binary::irProto::TimingSectionIrHeader eof;
    fromJson(*eofIt, eof);
    section.setEoF(eof);
  }
}

//---------------------------------------------------------------------------
// IrProto
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const binary::irProto::IrProto &proto)
{
  out["ClockPeriod"] = proto.getClockPeriod();

  ordered_json sectionsArr = ordered_json::array();
  for (int i = 0; i < proto.getSectionCount(); i++) {
    ordered_json sectionJson;
    toJson(sectionJson, proto.accessSection(i));
    sectionsArr.push_back(sectionJson);
  }
  out["Sections"] = sectionsArr;
}

void fromJson(const ordered_json &in, binary::irProto::IrProto &proto)
{
  std::vector<binary::irProto::TimingSection> sections;
  auto sectionsIt = in.find("Sections");
  if (sectionsIt != in.end()) {
    for (const auto &sectionJson : *sectionsIt) {
      binary::irProto::TimingSection section;
      fromJson(sectionJson, section);
      sections.push_back(section);
    }
  }
  proto = binary::irProto::IrProto(in.value("ClockPeriod", uint16_t(26316)),
      sections);
}

}
}
}
