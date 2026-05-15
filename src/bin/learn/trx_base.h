// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <vector>

namespace binary {
namespace trx {

class Base
{
  public:
    enum class Status
    {
      OK,
      DONE,
      ERR_SIZE,
      ERR_UNKNOWN_RETURNCODE, //received unknown return code from remote, see ProtocolStatus
      ERR_RESPONSE_FORMAT,    //response heder, layout format/values not as expected
      ERR_PAYLOAD_FORMAT,     //dito, payload
      ERR_TERM,
      ERR_TIMEOUT,
      ERR_UNKNOWN
    };

  protected:
    std::vector<uint16_t> payload;
    double clock = 0;
    std::vector<uint16_t> excess;

    //static values from child classes
    virtual int getHeaderMinSize() = 0;
    virtual int getProtoclCmd() = 0;
    virtual int getProtocolRespByte3() = 0;

    //assume status is byte data[2] in response
    enum class ProtocolStatus
    {
      OK = 0x01,
      TIMEOUT = 0x02
    };

    void moveExcessBytes(bool check);

  public:
    /** generate request */
    virtual std::vector<uint8_t> get() = 0;

    /** check data block */
    virtual Status check(const std::vector<uint8_t> &data);

    /** get uninterpretet error byte */
    uint8_t getErrorByte(const std::vector<uint8_t> &data);

    /**
     * append data block
     *
     * @param data response data block
     * @param first this is the first ever block in this transfer (command single/stream doesn't matter)
     * @return see 'Status'
     */
    virtual Status addChunk(const std::vector<uint8_t> &data, bool first) { return Status::OK; };

    /** access payload. size() == 0 -> nothing available */
    virtual const std::vector<uint16_t> &getPayload()
    {
      return payload;
    }

    /** get IR clock */
    virtual const double getClock()
    {
      return clock;
    }

    /** read non-timing related words. word = unknown use */
    virtual const std::vector<uint16_t> &getExcess()
    {
      return excess;
    }

};

}
}
