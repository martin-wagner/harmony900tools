// SPDX-License-Identifier: LGPL-2.1-or-later

#include "codeJson.h"

#include <QByteArray>

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
  std::vector<uint16_t> markPause;

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

void toJson(ordered_json &out, const binary::irProto::Section &section)
{
  out["Index"] = section.getIndex();
  out["Data"] = lib::bitsToHexString(section.getData());
  out["DataBitCount"] = section.getData().size();
}

void fromJson(const ordered_json &in, binary::irProto::Section &section)
{
  std::vector<bool> data;

  int index = in.value("Index", 0);
  auto dataIt = in.find("Data");
  if (dataIt != in.end()) {
    size_t bitCount = in.value("DataBitCount", size_t(0));
    data = lib::hexStringToBits(dataIt->get<std::string>(), bitCount);
  }
  section = binary::irProto::Section(index, data);
}

void toJson(ordered_json &out, const binary::irProto::Code &code)
{
  out["Index"] = code.getIndex();
  out["Delay"] = code.getDelay();
  out["RepeatFrame"] = code.getRepeatFrame();
  out["DataFrameTxCount"] = code.getDataFrameTxCount();
  out["Control"] = code.getControl();

  ordered_json sectionsArr = ordered_json::array();
  for (const auto &section : code.accessSections()) {
    ordered_json sectionJson;
    toJson(sectionJson, section);
    sectionsArr.push_back(sectionJson);
  }
  out["Sections"] = sectionsArr;
}

void fromJson(const ordered_json &in, binary::irProto::Code &code)
{
  code.setIndex(in.value("Index", uint16_t(0)));
  code.setDelay(in.value("Delay", uint16_t(0)));
  code.setRepeatFrame(in.value("RepeatFrame", uint8_t(0)));
  code.setDataFrameTxCount(in.value("DataFrameTxCount", uint8_t(1)));
  code.setControl(in.value("Control", uint8_t(0)));

  std::vector<binary::irProto::Section> sections;
  auto sectionsIt = in.find("Sections");
  if (sectionsIt != in.end()) {
    for (const auto &sectionJson : *sectionsIt) {
      binary::irProto::Section section(0, { });
      fromJson(sectionJson, section);
      sections.push_back(section);
    }
  }
  code.setSections(sections);
}

}
}
}
