// SPDX-License-Identifier: LGPL-2.1-or-later

#include "irProtoJson.h"
#include "document/data/enum.h"

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

ordered_json toJsonItemList(
    const binary::irProto::TimingSectionIrPayload::Quantum &items)
{
  ordered_json arr = ordered_json::array();
  for (const auto &item : items) {
    ordered_json itemJson;
    toJson(itemJson, item);
    arr.push_back(itemJson);
  }
  return arr;
}

binary::irProto::TimingSectionIrPayload::Quantum fromJsonItemList(
    const ordered_json &in)
{
  binary::irProto::TimingSectionIrPayload::Quantum items;
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
  ordered_json quanta = ordered_json::array();

  for (int i = 0; i < payload.getQuantisation(); i++) {
    quanta.push_back(toJsonItemList(payload.accessQuantum(i)));
  }

  out["Quanta"] = quanta;
}

void fromJson(const ordered_json &in,
    binary::irProto::TimingSectionIrPayload &payload)
{
  payload = binary::irProto::TimingSectionIrPayload();

  auto quantaIt = in.find("Quanta");
  if (quantaIt == in.end() || !quantaIt->is_array()) {
    return;
  }

  uint8_t pos = 0;
  for (const auto &quantumJson : *quantaIt) {
    auto timings = fromJsonItemList(quantumJson);
    payload.setQuantum(pos, timings);
    pos++;
  }
}

void fromJsonOld(const ordered_json &in,
    binary::irProto::TimingSectionIrPayload &payload)
{
  int pos = 0;
  payload = binary::irProto::TimingSectionIrPayload();

  auto falseIt = in.find("FalsePair");
  if (falseIt != in.end()) {
    auto timings = fromJsonItemList(*falseIt);
    payload.setQuantum(pos, timings);
    pos++;
  }

  auto trueIt = in.find("TruePair");
  if (trueIt != in.end()) {
    auto timings = fromJsonItemList(*trueIt);
    payload.setQuantum(pos, timings);
  }
}

//---------------------------------------------------------------------------
// TimingSection
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const binary::irProto::TimingSection &section)
{
  out["BitCount"] = section.getBitCount();
  out["Toggle"] = section.getToggle();
  out["Timing"] = section.getTiming();

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

  auto sofIt = in.find("SoF");
  if (sofIt != in.end()) {
    binary::irProto::TimingSectionIrHeader sof;
    fromJson(*sofIt, sof);
    section.setSoF(sof);
  }

  auto dataIt = in.find("Data");
  if (dataIt != in.end()) {
    if (in.contains("Ctrl0")) {
      //fixme pre v0.2 when to drop?
      //ctrl0 / ctrl1 contain redundant data, ignore those
      binary::irProto::TimingSectionIrPayload data;
      fromJsonOld(*dataIt, data);
      section.setData(data);

    } else {
      binary::irProto::TimingSectionIrPayload data;
      fromJson(*dataIt, data);
      section.setData(data);
    }
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

void toJson(ordered_json &out, const binary::irProto::IrProto &proto,
    const string &name)
{
  out["Name"] = name;
  out["ClockPeriod"] = proto.getClockPeriod();

  ordered_json sectionsArr = ordered_json::array();
  for (int i = 0; i < proto.getSectionCount(); i++) {
    ordered_json sectionJson;
    toJson(sectionJson, proto.accessSection(i));
    sectionsArr.push_back(sectionJson);
  }
  out["Sections"] = sectionsArr;
}

string fromJson(const ordered_json &in, binary::irProto::IrProto &proto)
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
  return in.value("Name", data::Enum(data::CodeType::Proprietary).getString());
}

}
}
}
