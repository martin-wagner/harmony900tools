// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef UNIT_TEST
#define IRREMOTEESP8266_DEFINED_UNIT_TEST
#define UNIT_TEST //make IRemoteESP8266 run without the ESP8266...
#endif
#include <IRrecv.h>
#include <IRutils.h>
#ifdef IRREMOTEESP8266_DEFINED_UNIT_TEST
#undef UNIT_TEST
#undef IRREMOTEESP8266_DEFINED_UNIT_TEST
#endif

#include "decode.h"
#include "bin/irProto/code.h"

using namespace std;

namespace binary
{
namespace codec
{

Decode::Decode()
{
}

Decode::Decode(const TimingStream &data, int carrier) :
    Decode()
{
  parse(data, carrier);
}

Status Decode::parse(TimingStream data, int carrier)
{
  IRrecv receiver(0, 1000);
  res = IrDecodeResult();

  volatile irparams_t *params = receiver._getParamsPtr();
  if (params == nullptr) {
    res.decoded = Status::ERROR_LIB;
    res.error = "Lib error";
    return res.decoded;
  }
  auto raw = data.convertMarkPause();
  if ((raw.size() < MIN_SIZE) || (raw.size() + 1 > params->bufsize)) {
    res.decoded = Status::ERROR_SIZE;
    res.error = "too small/large";
    return res.decoded;
  }

  /*
   * IRremoteESP8266 uses 2 us ticks internally.
   *
   * rawbuf[0] is unused. The first timing starts at rawbuf[1].
   */
  params->rawbuf[0] = 0;
  for (size_t i = 0; i < raw.size(); i++) {
    params->rawbuf[i + 1] = raw[i] / kRawTick;
    if (raw[i] > REPEAT_FRAME_THRESHOLD_us) {
      break;
    }
  }

  params->rawlen = static_cast<uint16_t>(raw.size() + 1);
  params->overflow = false;
  params->rcvstate = kStopState;

  decode_results decoder { };

  /*
   * With UNIT_TEST enabled, IRrecv::decode() doesn't populate these
   * fields itself. Normally the hardware receive path does that.
   */
  decoder.rawbuf = params->rawbuf;
  decoder.rawlen = params->rawlen;
  decoder.overflow = params->overflow;
  if (!receiver.decode(&decoder)) {
    res.decoded = Status::ERROR_PAYLOAD_FORMAT;
    res.error = "No protocol or unsupported data";
    return res.decoded;
  }

  //pull the decoded data. can be changed for various reasons further down.
  res.codeString = typeToString(decoder.decode_type);
  res.address = decoder.address;
  res.command = decoder.command;
  res.data = irProto::Code::u64tobits(decoder.bits, decoder.value);
  res.decoded = Status::OK;
  //only allow the protocols we support encoding for
  //functions must match "encode.cpp"
  switch (decoder.decode_type) {
    case decode_type_t::NEC:
      if (decoder.bits == 32) {
        res.codeType = CodeType::NEC;
      }
      break;
    case decode_type_t::NEC_LIKE:
      if (decoder.bits == 32) {
        res.command = decoder.value & 0xffff;
        res.address = (decoder.value >> 16) & 0xffff;
        res.codeType = CodeType::NEC;
      }
      break;
    case decode_type_t::PANASONIC:
    case decode_type_t::DENON:
      if (decoder.bits == 48) {
        //address = "mnf"
        res.command = 0;
        //data contains among others: 8bit device, 8bit subdevice, 8bit command
        res.codeType = CodeType::KASEIKYO;
      }
      break;
    case decode_type_t::SONY:
      if (decoder.bits == 12) {
        res.codeType = CodeType::SIRCS12;
      } else if (decoder.bits == 15) {
        res.codeType = CodeType::SIRCS15;
      } else if (decoder.bits == 20) {
        res.codeType = CodeType::SIRCS20;
      }
      break;
    case decode_type_t::SAMSUNG:
      if (decoder.bits == 32) {
        res.codeType = CodeType::Samsung32;
      }
      break;
    case decode_type_t::RC5:
    case decode_type_t::RC5X:
      res.codeType = CodeType::PhilipsRC5;
      break;
    case decode_type_t::RC6:
      if (decoder.bits == 20) {
        res.codeType = CodeType::PhilipsRC6;
        res.address = res.address & 0xff; //mask header / toggle
      } else if (decoder.bits == 36) {
        res.codeType = CodeType::PhilipsRC6A;
        res.address = res.address & 0x7FFF7F; //mask always-1-bit / header / toggle
        res.data = irProto::Code::u64tobits(31, decoder.value);
      }
      break;
    default:
      res.decoded = Status::ERROR_UNSUPPORTED;
      res.error = "Protocol detected, but not supported";
  }
  return res.decoded;
}

}
}

