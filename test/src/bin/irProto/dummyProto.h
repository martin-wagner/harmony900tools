// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "bin/irProto/code.h"


namespace {

using namespace binary;
using namespace irProto;
using Symbols = std::vector<uint8_t>;

/** section with <symbols> symbols and <quanta> states per symbol (0 = no payload) */
inline TimingSection makeSection(uint16_t symbols, uint8_t quanta)
{
  TimingSectionIrPayload payload;

  for (uint8_t q = 0; q < quanta; q++) {
    std::vector<Item> timings = { { true, 560 }, { false, static_cast<uint16_t>(560 * (q + 1)) } };
    payload.setQuantum(q, timings);
  }

  TimingSection section;
  section.setBitCount(symbols);
  section.setData(payload);
  return section;
}

inline IrProto makeProto(const std::vector<TimingSection> &sections)
{
  return IrProto(27778, sections);
}

// data section + repeat frame without data (NEC style)
inline IrProto protoNec()
{
  return makeProto( { makeSection(32, 2), makeSection(0, 0) });
}

// one section, 2 states per symbol
inline IrProto protoSingle(uint16_t symbols)
{
  return makeProto( { makeSection(symbols, 2) });
}

// 4 / 1 / 16 symbols
inline IrProto protoRc6()
{
  return makeProto( { makeSection(4, 2), makeSection(1, 2), makeSection(16, 2) });
}

inline IrProto protoMulti16()
{
  return makeProto( { makeSection(16, 2), makeSection(16, 2), makeSection(16, 2) });
}

inline IrProto protoBelgacom()
{
  return makeProto( { makeSection(16, 4), makeSection(16, 4), makeSection(16, 4) });
}

inline IrProto protoBeoDual()
{
  return makeProto( { makeSection(17, 4), makeSection(1, 2), makeSection(17, 4), makeSection(20, 4) });
}

inline IrProto protoGalaxis()
{
  return makeProto( { makeSection(8, 4), makeSection(1, 2), makeSection(7, 4), makeSection(1, 2) });
}

}
