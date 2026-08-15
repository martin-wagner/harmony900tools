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

    using CodeType = document::data::CodeType;

    struct IrDecodeResult
    {
        Status decoded = Status::ERROR_UNKNOWN;
        CodeType codeType = CodeType::Unknown;
        std::string codeString = "Unknown";
        int device = -1;
        int subDevice = -1;
        int command = -1;
        int subCommand = -1;
        int hex[4]; //don't use, debugging only
        std::vector<bool> data;
        std::string misc;
        std::string error;
    };

    static constexpr int REPEAT_FRAME_THRESHOLD_us = 10000;

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

    IrDecodeResult decodeIrTimings(const std::vector<uint16_t> &raw, int carrierHz);

    //reimplements DecodeIr()
    void myDecodeIr(unsigned int* Context, int* TpaiBursts, int TiFreq, int TiSingleBurstCount,
        int TiRepeatBurstCount, char* TsProtocol, int* TiDevice, int* TiSubDevice, int* TiOBC,
        int* TaiHex, char* TsMisc, char* TsError, int &bitCount, std::vector<uint8_t> &data);
};

}
}
