// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "file.h"

namespace binary {
namespace irProto {

/** single timing section */
class Section
{
  protected:
    int index;
    std::vector<bool> data;

  public:
    Section(int index, const std::vector<bool> &data) :index(index), data(data)
    {
    }

    int getIndex() const { return index; }
    const std::vector<bool> &getData() const { return data; };
};


/** decode "code" field from xml */
class Code
{
  protected:
    enum class Ctrl : uint8_t {
      FLAT = 0,
      SECTIONS_1 = 1,
      SECTIONS_N, // 2, 3...
    };

    static constexpr int HEADER_SIZE = 8;
    static constexpr int FOOTER_SIZE = 1;
    static constexpr double SYSCLOCK = 18000000.0;

    uint16_t index = 0;
    uint16_t ticks = 0;
    uint8_t dataSectionCount = 0;
    uint8_t haveRepeatFrame = 0;
    uint8_t dataFrameTxCount = 1;
    Ctrl ctrl = Ctrl::FLAT;
    std::vector<Section> sections;

    std::vector<bool> bytesToBits(const std::vector<uint8_t>& data);
    std::vector<uint8_t> bitsToBytes(const std::vector<bool>& bits);
    //bit-stream + end
    Status parseFlat(const std::vector<uint8_t>& data);
    //bit-stream
    Status parseSingleSection(const std::vector<uint8_t>& data);
    //data section count 2byte bit-streams
    Status parseMultiSection(const std::vector<uint8_t>& data);

  public:
    /** create empty code item */
    Code();
    /** parse code */
    Code(const std::string &code);

    /** parse code (overwrite existing) */
    Status parse(const std::string &code);

    /** irProto protocol index */
    int getIndex() const { return index; };
    void setIndex(uint16_t v) { index = v; }

    /** payload type selection byte */
    uint8_t getControl() const { return static_cast<uint8_t>(ctrl); };
    void setControl(uint8_t v) { ctrl = static_cast<Ctrl>(v); }

    /** payload type selection byte */
    std::string getControlStr() const
    {
      switch (ctrl) {
        case Ctrl::FLAT:
          return "Flat";
        case Ctrl::SECTIONS_1:
          return "Single Section";
        default:
          return "Multi Section (" + std::to_string(static_cast<uint8_t>(ctrl)) + ")";
      }
    };

    /** irProto protocol ticks */
    int getTicks() const { return ticks; };
    void setTicks(uint16_t v) { ticks = v; }

    /** get carrier clock */
    double getClock() const { return (SYSCLOCK / ticks); };

    std::string getRepeatTypeStr() const
    {
      if (haveRepeatFrame) {
        return "Repeat Frame";
      }
      return "Repeats: " + std::to_string(dataFrameTxCount);
    }

    uint8_t getDataSectionCount() const { return dataSectionCount; }
    void setDataSectionCount(uint8_t v) { dataSectionCount = v; }

    uint8_t getRepeatFrame() const { return haveRepeatFrame; }
    void setRepeatFrame(uint8_t v) { haveRepeatFrame = v; }

    uint8_t getDataFrameTxCount() const { return dataFrameTxCount; }
    void setDataFrameTxCount(uint8_t v) { dataFrameTxCount = v; }

    const std::vector<Section> &accessSections() const { return sections; }
    void setSections(const std::vector<Section> &s) { sections = s; }

    /** create data item for coding with irProto data */
    const IrProto::Data getData() const;
};

}
}
