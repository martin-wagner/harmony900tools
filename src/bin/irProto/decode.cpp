// SPDX-License-Identifier: LGPL-2.1-or-later

#include <boost/algorithm/string/predicate.hpp>
#include <set>
#include <DecodeIR.h>

#include "lib/bits.h"
#include "decode.h"

using namespace std;

namespace binary
{
namespace irProto
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
  res = decodeIrTimings(data.convertMarkPause(), carrier);
  if (res.decoded == Status::OK) {
    if (boost::istarts_with(res.codeString, "NEC")) {
      res.codeType = CodeType::NEC;
      auto subCommand = lib::reverseBits(static_cast<uint8_t>(res.hex[0]));
      if (res.command != (subCommand ^ 0xff)) {
        res.subCommand = subCommand;
      }
    } else if (boost::iequals(res.codeString, "Tivo")) {
      res.codeType = CodeType::NEC;
    } else if (boost::iequals(res.codeString, "Apple")) {
      res.codeType = CodeType::NEC;
    } else if (boost::iequals(res.codeString, "KASEIKYO")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "Panasonic")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "JVC-48")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "Denon-K")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "Fujitsu")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "SharpDVD")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "Teac-K")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "Mitsubishi-K")) {
      res.codeType = CodeType::KASEIKYO;
    } else if (boost::iequals(res.codeString, "Sony")) {
      res.codeType = CodeType::SIRCS;
    } else if (boost::iequals(res.codeString, "Samsung")) {
      res.codeType = CodeType::Samsung32;
    } else if (boost::iequals(res.codeString, "rc5")) {
      res.codeType = CodeType::PhilipsRC5;
    } else if (boost::iequals(res.codeString, "rc5x")) {
      res.codeType = CodeType::PhilipsRC5;
    } else if (boost::iequals(res.codeString, "rc6")) {
      res.codeType = CodeType::PhilipsRC6;
    }
  }
  return res.decoded;
}

Decode::IrDecodeResult Decode::decodeIrTimings(const vector<uint16_t> &raw,
    int carrierHz)
{
  IrDecodeResult result;

  // DecodeIR wants alternating pulse/space values as int, starting with a pulse.
  vector<int> bursts(raw.size());
  for (size_t i = 0; i < raw.size(); i++) {
    bursts[i] = static_cast<int>(raw[i]);
    if (bursts[i] > REPEAT_FRAME_THRESHOLD_us) {
      //we want only the first frame, no repeats
      break;
    }
  }

  if (bursts.size() < 4) {
    result.decoded = Status::ERROR_SIZE;
    result.error = "raw timing stream must have at least four pulses";
    return result;
  }
  if ((bursts.size() % 2) != 0) {
    bursts.push_back(REPEAT_FRAME_THRESHOLD_us); //append silence to the end
  }
  int burstCount = static_cast<int>(bursts.size()) / 2; // one burst = pulse + space

  //fill DecodeIR input params
  unsigned int context[2] = { 0, 0 };
  int device = -1;    // sentinel: no extra-burst info supplied
  int subDevice = -1; // sentinel: default context length
  int obc = -1;
  int hex[4] = { -1, -1, -1, -1 };

  char protocolBuf[256] = { 0 };
  char miscBuf[256] = { 0 };
  char errorBuf[256] = { 0 };

  int bitCount = 0;
  vector<uint8_t> bitData;

  myDecodeIr(context, bursts.data(), carrierHz, burstCount, // single (non-repeat) burst count
      0,          // repeat burst count - none
      protocolBuf, &device, &subDevice, &obc, hex, miscBuf, errorBuf, bitCount, bitData);

  if (protocolBuf[0] != '\0') {
    result.decoded = Status::OK;
    result.codeString = protocolBuf;
    result.device = device;
    result.subDevice = subDevice;
    result.command = obc;
    for (int i = 0; i < 4; i++) {
      result.hex[i] = hex[i];
    }



    result.misc = miscBuf;
  }
  result.error = errorBuf;
  return result;
}

void Decode::myDecodeIr(unsigned int *Context, int *TpaiBursts, int TiFreq,
    int TiSingleBurstCount, int TiRepeatBurstCount, char *TsProtocol,
    int *TiDevice, int *TiSubDevice, int *TiOBC, int *TaiHex, char *TsMisc,
    char *TsError, int &bitCount, vector<uint8_t> &data)
{
  //we reimplement DecodeIr().
  //DecodeIr gives us a nice representation of the received data, decoded to
  //a high detail level (mnf, command, ..., manufacturer specific data shifts, ...).
  //At the first detection stage, we don't want any of that, but:
  //"protocol type, data bit count, data bits"
  //this gives us access to the inner "Signal" class

  //copy/paste as is
  if (TiSingleBurstCount + TiRepeatBurstCount >= MinFrame) {
    char szMessage[500];
    sprintf(szMessage,
        "N=%d SCount=%d RCount=%d Burst1=%d Freq=%d hex[0]=%d Error=\"%s\"",
        *Context, TiSingleBurstCount, TiRepeatBurstCount, TpaiBursts[1], TiFreq,
        TaiHex[-1], TsError);
    Signal _Signal(Context, TpaiBursts, TiFreq, TiSingleBurstCount,
        TiRepeatBurstCount, TsProtocol, TiDevice, TiSubDevice, TiOBC, TaiHex,
        TsMisc, TsError);

    _Signal.decode2();

    //get our data out
    bitCount = _Signal.nBit;
    for (int i = 0; i < sizeof(_Signal.cBits) / sizeof(_Signal.cBits[0]); i++) {
      data.push_back(_Signal.cBits[i]);
    }
  }
  return;
}

}
}

