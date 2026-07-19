// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "cli/lib/lib.h"
#include "bin/timing.h"

namespace binary {
namespace ssIr {

/** UR Default Carrier clock. Use this if not available, will work 95% of times.
 * Receiver bandwidth normally is so wide this will work with 36...40kHz. */
const double DEFAULT_CLOCK_HZ = 38000.0;

/** File handling status */
enum class Status {
  OK,
  ERROR_FILE,
  ERROR_SIZE,
  ERROR_FILE_FORMAT,
  ERROR_PAYLOAD_FORMAT,
  ERROR_UNKNOWN
};

/** One Item in SsIr.bin file */
class SerialStreamIr
{
  protected:
    uint16_t clockPeriod = 26316;//ns -- 38kHz
    uint16_t filler = 0;
    TimingStream stream;

    static constexpr int HEADER_SIZE = 3;

    void setClock(double clock);

  public:
    SerialStreamIr();
    /** add a stream, alternating mark<us>, pause<us> + clock */
    SerialStreamIr(const std::vector<uint16_t> &data, double clock /* hz */);
    /** add a stream + clock */
    SerialStreamIr(TimingStream &stream, double clock /* hz */);
    /** add a raw stream from SsIr.bin file */
    SerialStreamIr(std::vector<uint16_t> data);

    void addData(std::vector<uint16_t> &data);
    const TimingStream &accessStream() const { return stream; };
    double getClock() const { return 1.0 / static_cast<double>(clockPeriod) * 1000000000.0; };

    /** raw clock period in ns */
    uint16_t getClockPeriod() const { return clockPeriod; }
    void setClockPeriod(uint16_t period) { clockPeriod = period; }

    std::vector<uint16_t> serialise() const;
};

/** read/write SsIr.bin file */
class File
{
  protected:
    inline static const std::vector<uint8_t> header = { 0x01, 0x01, 0x05, 0x00, 0x00, 0x01 };
    static constexpr int MIN_FILE_SIZE = 8; //header + 0x01 + array count
    static constexpr int HEADER_SIZE = 5; //last 0x01 of header above belongs to payload with unknown use.

    std::vector<SerialStreamIr> streams;

  public:
    /** create empty file */
    File();
    /** parse file */
    File(const std::string &filename);

    /** parse file (overwrite existing) */
    Status parse(const std::string &filename);

    /** read stream count */
    int getStreamCount() const { return streams.size(); };
    bool isEmpty() const { return streams.empty(); };

    /** access a single stream (e.g. for human-readable printout) */
    const SerialStreamIr &accessStream(int index) const;

    /**
     * add a stream, alternating mark<us>, pause<us>
     * @param index [out] position where stream was added
     */
    Status appendStream(std::vector<uint16_t> &data, double clock, int &index);

    /** add a ready-to-use stream, e.g. from irlearn. takes ownership!
     * @param index [out] position where stream was added
     */
    void appendStream(TimingStream &stream, double clock, int &index);

    /** add a ready-to-use, fully-built stream
     * @param index [out] position where stream was added
     */
    void appendStream(const SerialStreamIr &stream, int &index);

    /** insert a stream
     *
     * @param stream stream ready-to-use, e.g. from irlearn. takes ownership!
     * @param index where to insert
     * @return true = ok
     */
    bool insertStream(TimingStream &stream, double clock, int index);

    /** remove a stream */
    void removeStream(int index);

    /** serialise SsIr.bin file and place it in vector */
    std::vector<uint8_t> serialise() const;

    /** serialise SsIr.bin file and write file */
    Status serialise(const std::string &filename) const;

  private:
    Status parseObject(const std::vector<uint8_t> &raw, uint16_t startOffset, uint16_t endOffset);

};

}
}


