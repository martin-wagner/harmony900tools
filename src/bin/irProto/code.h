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
  public:
    enum class Ctrl : uint8_t {
      FLAT = 0,
      SECTIONS_1 = 1,
      SECTIONS_N, // 2, 3...
    };

    static constexpr uint16_t DEFAULT_DELAY = 500;
    static constexpr uint16_t DEFAULT_REPEATS = 3;

  protected:
    static constexpr int HEADER_SIZE = 8;
    static constexpr int FOOTER_SIZE = 1;

    uint16_t index = 0;
    uint16_t delay = DEFAULT_DELAY; //ms, most likely. Unable to find what this is or where it applies. Most devices have 500, seen 0 ... 1500 in steps of 100.
    uint8_t haveRepeatFrame = 0;
    uint8_t dataFrameTxCount = 1;
    Ctrl ctrl = Ctrl::FLAT;
    std::vector<Section> sections;

  public:
    static std::vector<bool> bytesToBits(const std::vector<uint8_t>& data);
    static std::vector<bool> u64tobits(uint8_t bitCount, uint64_t data);
    static std::vector<uint8_t> bitsToBytes(const std::vector<bool>& bits);
    static uint64_t bitsToU64(const std::vector<bool>& bits);

  protected:
    //bit-stream + end
    Status parseFlat(const std::vector<uint8_t>& data);
    //bit-stream
    Status parseSingleSection(const std::vector<uint8_t>& data);
    //data section count 2byte bit-streams
    Status parseMultiSection(uint8_t expectedSectionCount, std::vector<uint8_t> data);

    void writeSections(std::vector<uint8_t> &data) const;

  public:
    /** create empty code item */
    Code();
    /** parse code */
    Code(const std::vector<uint8_t> &code);
    Code(const std::string &code);

    /** parse code (overwrite existing) */
    Status parse(std::vector<uint8_t> code);
    Status parse(const std::string &code);

    /** create code items from data. manually set either of setDataFrameTxCount / setRepeatFrame afterwards! */
    void createFlat(uint8_t index, int delay, uint8_t bits, uint64_t data);
    void createSingleSection(uint8_t index, int delay, uint8_t bits, uint64_t data);
    void createMultiSection(uint8_t index, int delay, const std::vector<std::pair<uint8_t, uint16_t>> &data);

    /** create code */
    std::string serialiseStr() const;
    std::vector<uint8_t> serialiseVec() const;

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

    /** Delay */
    int getDelay() const { return delay; };
    void setDelay(uint16_t v) { delay = v; }

    std::string getRepeatTypeStr() const
    {
      if (haveRepeatFrame) {
        return "Repeat Frame";
      }
      return "Repeats: " + std::to_string(dataFrameTxCount);
    }

    uint8_t getDataSectionCount() const { return sections.size(); }

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
