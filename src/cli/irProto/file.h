/*
 * file.h
 *
 *  Created on: Apr 4, 2026
 *      Author: martin
 */


#pragma once

#include "cli/lib/lib.h"
#include "cli/lib/data.h"

namespace irProto {

/** UR Default Carrier clock. Use this if not available, will work 95% of times.
 * Receiver bandwidth normally is so wide this will work with 36...40kHz. */
const double DEFAULT_CLOCK_HZ = 38000.0;

/** File handling status */
enum class Status {
  OK,
  ERROR_FILE,
  ERROR_OUTER_SIZE,        //file wrapper size
  ERROR_OUTER_CRC,         //file wrapper crc
  ERROR_OUTER_FILE_FORMAT, //file wrapper format
  ERROR_SIZE,
  ERROR_INDEX,
  ERROR_FILE_FORMAT,
  ERROR_PAYLOAD_FORMAT,
  ERROR_UNKNOWN
};

// one timing item -> is mark / timing in us
using Item = std::pair<bool, uint16_t>;

/** IR protocol part: SoF, EoF */
class TimingSectionIrHeader
{
  protected:
    std::vector<Item> stream;

  public:
    /** create an empty header */
    TimingSectionIrHeader();
    /** add a header block from IrProto.bin*/
    TimingSectionIrHeader(std::vector<uint8_t> data);

    /** add single timing item */
    void addItem(Item item);

    /** get count of timing items */
    int getCount() const { return stream.size(); };
    /** check container is empty */
    bool isEmpty() const { return stream.empty(); };

    /** access data */
    const std::vector<Item> & accessStream() const { return stream; };

    /** serialise for writing to disk */
    std::vector<uint8_t> serialise() const;

    /** serialise for creating timing stream */
    void serialiseIrStream(std::vector<Item> &out) const;
};

/** IR protocol part: Payload data encoding */
class TimingSectionIrPayload
{
  protected:
    std::vector<Item> streamFalse;
    std::vector<Item> streamTrue;

  public:
    /** create an empty block */
    TimingSectionIrPayload();
    /** add a block from IrProto.bin*/
    TimingSectionIrPayload(const std::vector<uint8_t> &data);

    /** set data */
    void setFalsePair(std::vector<Item> timings);
    void setTruePair(std::vector<Item> timings);

    /** access data */
    const std::vector<Item> & accessFalsePair() const { return streamFalse; };
    const std::vector<Item> & accessTruePair() const { return streamTrue; };

    bool haveFalse() const { return (!streamFalse.empty()); };
    bool haveTrue() const { return (!streamTrue.empty()); };
    bool isEmpty() const { return (streamTrue.empty() && streamFalse.empty()); };

    /** serialise for writing to disk */
    std::vector<uint8_t> serialise() const;

    /** serialise for creating timing stream */
    void serialiseIrStream(bool booleanState, std::vector<Item> &out) const;
};

/**
 * One IR protocol, all params needed to hand in data and encode IR frames
 *
 * Each IR protocol can have multiple combos of timing. This might be
 * Data / Repeat frame, different timings within frame, ...
 *
 * for details see irProto.md.
 */
class TimingSection
{
  public:
    /** frame type */
    enum class Ctrl0 :uint8_t {
      IS_REPEAT_FRAME = 0,
      IS_DATA_FRAME = 2
    };
    /** data type for data frame */
    enum class Ctrl1 :uint8_t {
      NOT_DATA_FRAME = 0,
      NO_DATA = 0,
      DATA_ONE_PAIR = 1, //can only code one binary state. ??
      DATA_TWO_PAIRS = 2 //codes two binary states (true/false)
    };

    static constexpr uint32_t TOGGLE_NONE = 0xff;
    static constexpr uint32_t PAUSE_IN_EOF = 0xffffffff;

  protected:
    static constexpr int HEADER_SIZE = 16;
    uint16_t bitCount = 0;
    uint8_t togglePos = TOGGLE_NONE;
    uint32_t interval = PAUSE_IN_EOF; //us
    Ctrl0 ctrl0 = Ctrl0::IS_DATA_FRAME;
    Ctrl1 ctrl1 = Ctrl1::DATA_TWO_PAIRS;
    TimingSectionIrHeader sof;
    TimingSectionIrPayload data;
    TimingSectionIrHeader eof;

  public:
    /** create an empty block */
    TimingSection();
    /** add a block from IrProto.bin*/
    TimingSection(const std::vector<uint8_t> &data, int offset);

    void setBitCount(uint16_t count);
    void setToggle(uint16_t toggle = TOGGLE_NONE);
    void setTiming(uint32_t t_us = PAUSE_IN_EOF);
    void setCtrl0(Ctrl0 c);
    void setCtrl1(Ctrl1 c);
    void setSoF(const TimingSectionIrHeader &sof);
    void setData(const TimingSectionIrPayload &d);
    void setEoF(const TimingSectionIrHeader &eof);

    uint16_t getBitCount() const { return bitCount;};
    uint8_t getToggle() const { return togglePos;};
    bool hasToggle() const { return (togglePos != TOGGLE_NONE);};
    uint32_t getTiming() const { return interval;};
    Ctrl0 getCtrl0() const { return ctrl0;};
    Ctrl1 getCtrl1() const { return ctrl1;};
    const TimingSectionIrHeader &getSoF() const { return sof;};
    const TimingSectionIrPayload &getData() const { return data;};
    const TimingSectionIrHeader &getEoF() const { return eof;};

    /** serialise for writing to disk */
    std::vector<uint8_t> serialise(int offset) const;

    /** serialise for creating timing stream */
    void serialiseIrStream(std::vector<Item> &out, const std::vector<bool> &data) const;
};

/** One Item in IrProto.bin file */
class IrProto
{
  protected:
    static constexpr int HEADER_SIZE = 7;
    static constexpr uint8_t START = 0x01;       //assumption
    static constexpr uint8_t UNUSED = 0;         //assumption
    static constexpr uint8_t SEPARATOR = 0x32;   //assumption

    uint16_t clockPeriod = 26316;//ns -- 38kHz

    std::vector<TimingSection> sections;

  public:
    /** create an empty protocol */
    IrProto();
    /** add a protocol block from IrProto.bin*/
    IrProto(const std::vector<uint8_t> &data, int offset);

    /** read section count */
    int getSectionCount() const { return sections.size(); };
    bool isEmpty() const { return sections.empty(); };

    /** access a single section */
    const TimingSection &accessSection(int index) const;
    /** get carrier clock */
    double getClock() const { return 1.0 / static_cast<double>(clockPeriod) * 1000000000.0; };

    /** serialise for writing to disk */
    std::vector<uint8_t> serialise(int offset) const;

    /** input data section index + section data (if any) */
    using Data = std::vector<std::pair<int, std::vector<bool>>>;
    /** serialise for creating timing stream */
    void serialiseIrStream(std::vector<Item> &out, const Data &data) const;
};

/** read/write IrProto.bin file */
class File
{
  protected:
    const std::vector<uint8_t> header = { 0x01, 0x01, 0x05, 0x00, 0x00, 0x01 };
    static constexpr int WRAPPER_SIZE = 8;
    static constexpr int MIN_FILE_SIZE = WRAPPER_SIZE + 8; //file wrapper + header + 0x01 + array count
    static constexpr int HEADER_SIZE = 5; //last 0x01 of header above belongs to payload with unknown use.

    std::vector<IrProto> protocols;

    static std::vector<Item> compress(std::vector<Item> items);

  public:
    /** create empty file */
    File();
    /** parse file */
    File(const std::string &filename);

    /** parse file (overwrite existing) */
    Status parse(const std::string &filename);

    /** read protocol count */
    int getProtocolCount() const { return protocols.size(); };
    bool isEmpty() const { return protocols.empty(); };

    /** access a single protocol */
    const IrProto &accessProtocol(int index) const;

    /** append a protocol
     *
     * @param proto protocol ready-to-use
     * @return index of added protocol
     */
    uint16_t appendProtocol(const IrProto &proto);

    /** remove a protocol */
    void removeProtocol(int index);

    /** serialise IrProto.bin file and place it in vector */
    std::vector<uint8_t> serialise(uint32_t *crc = nullptr) const;

    /** serialise IrProto.bin file and write file */
    Status serialise(const std::string &filename, uint32_t *crc = nullptr) const;

    /** serialise to timing stream */
    Status serialiseIrStream(lib::TimingStream &out, uint16_t index, const IrProto::Data &data) const;

  private:
    Status parseObject(const std::vector<uint8_t> &raw, uint16_t startOffset, uint16_t endOffset);

};

}


