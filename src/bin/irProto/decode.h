// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "bin/timing.h"
#include "file.h"
#include "document/data/enum.h"
#include "document/files/protocols.h"

namespace binary {
namespace irProto {


/** decode timing stream to get info about used protocol and data
 *
 * assumption: all protocol start bits in ir_protocols.h have length
 * so that the start bit + / - tolerance can be identified fully
 *
 * protocols that are not within that list, but have matching start
 * bit length should be excluded by further decoding / errors.
 * */
class Decode
{
  public:
    using CodeType = document::data::CodeType;

  protected:
    CodeType codeType = CodeType::Unknown;
    const IrProto *prot = nullptr;
    std::vector<bool> payload;
    double timingTolerance = 0.2; //factor +/-
    double ratioTolerance = 0.2;  //abs +/-

  public:
    /** create empty Decoder item */
    Decode(const document::files::ProtocolCatalogue &cat);
    /** Do decode */
    Decode(const document::files::ProtocolCatalogue &cat, const TimingStream &data);

    /** set timing tolerance */
    void setTolerance(double v) { timingTolerance = v; };

    /** parse Decode (overwrite existing) */
    Status parse(TimingStream data, bool headerOnly = false);

    /** get detected code type */
    CodeType getCodeType() const;

    /** create data item for coding with irProto data.
     *
     * @remark time-on-wire last => lsb => vec[0], independent of protocol type!
     */
    const std::vector<bool> getData() const;

  protected:
    void buildData(const document::files::ProtocolCatalogue &cat);
    bool checkTime(double expectedTime, double time);
    bool checkRatio(double expectedRatio, double ratio);

    //sorted by how unambiguously a protocol can be detected
    std::vector<std::pair<CodeType, IrProto>> protocols;

  protected:
    bool searchHeader(const TimingStream &data);
    Status searchData(const TimingStream &data);

    IrProto::Data createSearchData(uint64_t data);
    uint8_t checkData(const TimingStream &test, const TimingStream &check);
};

}
}
