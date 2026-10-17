// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "file.h"

namespace binary {
namespace irProto {

/** single timing section */
class Group
{
  protected:
    uint8_t section; //in corresponding IRproto
    uint8_t bits; // bits per symbol in data. Example: Boolean = 1 bit / 2 symbols (true/false), Hex = 4 bits / 16 symbols (0...F)
    std::vector<uint8_t> data; //list of quantums, size corresponding to IRproto

  public:
    /** default construt an empty item. this item can only be used as placeholder */
    Group() : section(0), bits(0) {};

    Group(uint8_t section, uint8_t bits, const std::vector<uint8_t> &data) :section(section), bits(bits), data(data)
    {
    }

    uint8_t getSection() const { return section; }
    uint8_t getBits() const { return bits; };
    const std::vector<uint8_t> &getData() const { return data; };
};

/** decode "code" field from xml */
class Code
{
  public:
    static constexpr uint16_t INVALID_INDEX_MARKER = 0xffff;
    static constexpr uint16_t DEFAULT_PRE_SEND_SILENCE = 500;
    static constexpr uint16_t DEFAULT_REPEATS = 3;
    static constexpr uint16_t HEADER_SIZE = 5;
    static constexpr uint16_t GROUPS_MIN_SIZE = 3; //min 3 "empty" markers
    static constexpr uint16_t MIN_SIZE = HEADER_SIZE + GROUPS_MIN_SIZE;

  protected:
    IrProto protocol;
    uint16_t index = INVALID_INDEX_MARKER;

    uint16_t delay;
    uint8_t repeats;
    std::vector<Group> start;
    std::vector<Group> repeat;
    std::vector<Group> finish;

  public:
    static std::vector<uint8_t> bytesToList(uint8_t bitCount, uint8_t bits, const std::vector<uint8_t>& data);
    static std::vector<uint8_t> u64bitsToList(uint8_t bitCount, uint64_t data);
    static std::vector<uint8_t> listToBytes(uint8_t bits, const std::vector<uint8_t> &values);
    static uint64_t listToU64bits(const std::vector<uint8_t> &values);

    static uint8_t getBitsPerSymbol(uint8_t quantums);
    static uint16_t getDataBytes(uint16_t symbols, uint8_t bitsPerSymbol);

  protected:
    Status parseGroup(std::vector<uint8_t> &data, std::vector<Group> &group);
    void setGroup(const std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &data, std::vector<Group> &group);
    void writeGroup(const std::vector<Group> &group, std::vector<uint8_t> &data) const;

  public:
    /** default construt an empty item. this item can only be used as placeholder */
    Code();
    /** create empty code item */
    Code(uint16_t index, const IrProto &protocol);
    /** parse code */
    Code(uint16_t index, const IrProto &protocol, const std::vector<uint8_t> &code);
    Code(uint16_t index, const IrProto &protocol, const std::string &code);

    /** wipe code */
    void reset();

    /** extract index from code */
    static Status parse(std::vector<uint8_t> code, int &index);
    static Status parse(const std::string &code, int &index);
    /** parse code (overwrite existing) */
    Status parse(std::vector<uint8_t> code);
    Status parse(const std::string &code);

    /** create common codes from data. You are responsible for respecting the protocol! */
    //rc5, japan, samsung, most others. one continuous data frame, repeat re-sends the frame.
    static Code createSimpleRepeatEqData(uint16_t index, const IrProto &protocol, uint16_t delay, uint8_t bits, uint64_t data, uint8_t repeats = DEFAULT_REPEATS);
    //NEC. one continious data frame, Repeat sends a special, empty repeat frame
    static Code createSimpleRepeatEmpty(uint16_t index, const IrProto &protocol, uint16_t delay, uint8_t bits, uint64_t data, uint8_t repeats = 1);
    //RC6. assembles a data frame from multiple sections, repeat re-sends the frame.
    static Code createMultiRepeatEqData(uint16_t index, const IrProto &protocol, uint16_t delay, const std::vector<std::pair<uint8_t, uint64_t>> &data, uint8_t repeats = DEFAULT_REPEATS);

    /** create code */
    std::string serialiseStr() const;
    std::vector<uint8_t> serialiseVec() const;

    /** irProto protocol index. the index is only ever used for (de)serialising and can be freely set */
    void changeIndex(uint16_t index) { this->index = index; };
    uint16_t getIndex() const { return index; };

    /** Delay */
    uint16_t getDelay() const { return delay; };
    void setDelay(uint16_t v) { delay = v; }

    /** Repeats */
    uint8_t getRepeats() const { return repeats; };
    void setRepeats(uint8_t v) { repeats = v; }

    /** accessors */
    const std::vector<Group> &accessStartGroups() const { return start; }
    const std::vector<Group> &accessRepeatGroups() const { return repeat; }
    const std::vector<Group> &accessFinishGroups() const { return finish; }
    std::vector<Group> &accessStartGroups()  { return start; }
    std::vector<Group> &accessRepeatGroups()  { return repeat; }
    std::vector<Group> &accessFinishGroups()  { return finish; }

    /** writers. group-list with protocol section + symbol list */
    void setStartGroups(const std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &data) { setGroup(data, start); }
    void setRepeatGroups(const std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &data) { setGroup(data, repeat); }
    void setFinishGroups(const std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &data) { setGroup(data, finish); }

    /** checks if a valid comand can be created */
    bool isValid() const;

    /** create data item for coding with irProto data */
    const IrProto::Data getData() const;
};

}
}
