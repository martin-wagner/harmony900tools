// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "bin/timing.h"
#include "file.h"
#include "document/data/enum.h"

namespace binary {
namespace irProto {


/** decode timing stream to get info about used protocol and data */
class Decode
{
  public:
    static constexpr int REPEAT_FRAME_THRESHOLD_us = 10000;
    static constexpr int MIN_SIZE = 4;
    using CodeType = document::data::CodeType;

    struct IrDecodeResult
    {
        Status decoded = Status::ERROR_UNKNOWN;
        CodeType codeType = CodeType::Unknown;
        std::string codeString = "Unknown";
        uint32_t address = -1;
        uint32_t command = -1;
        std::vector<bool> data;
        std::string error;
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
