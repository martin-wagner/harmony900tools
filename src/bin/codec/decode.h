// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "bin/timing.h"
#include "document/data/enum.h"

namespace binary {
namespace codec {

/** File handling status */
enum class Status {
  OK,
  ERROR_LIB,
  ERROR_SIZE,
  ERROR_PAYLOAD_FORMAT,
  ERROR_UNSUPPORTED,
  ERROR_UNKNOWN
};

/** decode timing stream to get info about used protocol and data */
class Decode
{
  public:
    static constexpr int REPEAT_FRAME_THRESHOLD_us = 10000;
    static constexpr int MIN_SIZE = 4;
    using CodeType = document::data::CodeType;

    struct IrDecodeResult
    {
        Status decoded = Status::ERROR_UNKNOWN; //decodeing sucessful?
        CodeType codeType = CodeType::Unknown;  //our code type
        std::string codeString = "Unknown";     //the libraries code type (as string)
        uint32_t address = -1;                  //decoded address
        uint32_t command = -1;                  //decoded command
        std::vector<bool> data;                 //raw data as received from remote
        std::string error;                      //info about !Status::OK
    };

  public:
    /** create empty Decoder item */
    Decode();
    /** Do decode */
    Decode(const TimingStream &data, int carrier);

    /** parse Decode (overwrite existing) */
    Status parse(TimingStream data, int carrier);

    /** get detected code type */
    CodeType getCodeType() const { return res.codeType; };

    /** get data */
    const IrDecodeResult &getData() const { return res; };

  protected:
    IrDecodeResult res;
};

}
}
