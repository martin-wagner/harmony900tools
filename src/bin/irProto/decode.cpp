// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/bits.h"
#include "decode.h"

using namespace std;

namespace binary
{
namespace irProto
{

Decode::Decode(const document::files::ProtocolCatalogue &cat)
{
  buildData(cat);
}

Decode::Decode(const document::files::ProtocolCatalogue &cat,
    const TimingStream &data) :
    Decode(cat)
{
  parse(data);
}

Status Decode::parse(TimingStream data, bool headerOnly)
{
  codeType = CodeType::Unknown;
  payload.clear();

  if (data.timings().empty()) {
    return Status::ERROR_SIZE;
  }
  auto haveHeader = searchHeader(data);
  if (!haveHeader) {
    return Status::ERROR_UNKNOWN;
  }
  if (headerOnly) {
    return Status::OK;
  }
  return searchData(data);
}

Decode::CodeType Decode::getCodeType() const
{
  return codeType;
}

const vector<bool> Decode::getData() const
{
  return payload;
}

void Decode::buildData(const document::files::ProtocolCatalogue &cat)
{
  auto names = document::data::Enum<CodeType>::toQStringList();
  for (const auto &name : names) {
    auto cmd = cat.get(name);
    if (cmd.isEmpty()) {
      continue;
    }
    auto type = document::data::Enum<CodeType>(name).getValue();
    protocols.push_back( { type, cmd });
  }
}

bool Decode::checkTolerance(double expectedTime, double time)
{
  auto rangeMax = expectedTime + expectedTime * tolerance;
  auto rangeMin = expectedTime - expectedTime * tolerance;

  if ((time < rangeMax) && (time > rangeMin)) {
    return true;
  }
  return false;
}

bool Decode::searchHeader(const TimingStream &data)
{
  double expectedStartBitTime;
  double expectedPauseBitTime;

  auto startBitTime = data.timings()[0].mark_us;
  auto startPauseTime = data.timings()[0].pause_us;

  for (const auto &protocol : protocols) {
    if (protocol.second.isEmpty()) {
      continue;
    }
    auto sof = protocol.second.accessSection(0).getSoF().accessStream();
    if (!sof.empty()) {
      //check SoF
      while (!sof.empty() && (sof.front().first == false)) {
        //can't be part of rx data
        sof.erase(sof.begin());
      }
      //check start "mark"
      if (!sof.empty()) {
        expectedStartBitTime = sof.front().second;
        if (!checkTolerance(expectedStartBitTime, startBitTime)) {
          continue;
        }
        //"mark" matches, store away and continue search for a better match
        codeType = protocol.first;
        prot = &protocol.second;

        sof.erase(sof.begin());
      }
      //check start "pause"
      //fixme this doesnt work with "manchester" coding when the pause is enlarged by the first data bit!
      if (!sof.empty() && (sof.front().first == false)) {
        expectedPauseBitTime = sof.front().second;
        if (checkTolerance(expectedPauseBitTime, startPauseTime)) {
          //match
          codeType = protocol.first;
          prot = &protocol.second;
          return true;
        }
      }
    } else {
      //no SoF. special checks.
      switch (protocol.first) {
        case CodeType::PhilipsRC5:
          //manchester. can start with a single/double-wide mark and continue with a single/double wide pause
          if ((checkTolerance(889, startBitTime)
              || checkTolerance(2 * 889, startBitTime))
              && (checkTolerance(889, startPauseTime)
                  || checkTolerance(2 * 889, startPauseTime))) {
            //wide range of validity, store away and continue search for a better match
            codeType = protocol.first;
            prot = &protocol.second;
          }
          break;
        default:
          break;
      }
    }
  }
  if (codeType != CodeType::Unknown) {
    return true;
  }
  return false;
}

Status Decode::searchData(const TimingStream &data)
{
  //- we know the protocol
  //- we can encode data for the protocol
  //--> let's just do a binary search for the matching data

  uint32_t i;
  uint64_t testData = 0;
  uint8_t testPosA; //pos in timing stream != bit pos !!
  uint8_t testPosB;
  TimingStream check;
  IrProto::Data tmp;
  auto bitCount = prot->getBitCount();

  for (i = 0; i < bitCount; i++) {
    //assumption: bit not set
    check = TimingStream();
    tmp = createSearchData(testData);
    prot->serialiseIrStream(check, tmp);
    testPosA = checkData(data, check);
    //assumption: bit set
    check = TimingStream();
    tmp = createSearchData(testData | (uint64_t(1) << i));
    prot->serialiseIrStream(check, tmp);
    testPosB = checkData(data, check);
    //check which one was right
    if (testPosB > testPosA) {
      testData = testData | (uint64_t(1) << i);
    }
  }

  payload = lib::u64ToBitsLsb(bitCount, testData);
  reverse(payload.begin(), payload.end());
  return Status::OK;
}

IrProto::Data Decode::createSearchData(uint64_t data)
{
  uint32_t i;
  IrProto::Data ret;

  for (i = 0; i < prot->getSectionCount(); i++) {
    const auto &section = prot->accessSection(i);
    if (section.getCtrl0() != TimingSection::Ctrl0::IS_DATA_FRAME) {
      continue;
    }
    auto bits = lib::u64ToBitsLsb(section.getBitCount(), data);
    ret.push_back( { i, move(bits) });
  }
  return ret;
}

uint8_t Decode::checkData(const TimingStream &test, const TimingStream &check)
{
  int pos = 0;

  try {
    for (pos = 0; pos < test.timings().size(); pos++) {
      const auto &testBlock = test.timings().at(pos);
      const auto &checkBlock = check.timings().at(pos);
      auto checkMark = checkTolerance(checkBlock.mark_us, testBlock.mark_us);
      auto checkPause = checkTolerance(checkBlock.pause_us, testBlock.pause_us);
      if (!checkMark || !checkPause) {
        //mismatch
        return pos;
      }
    }
  } catch (out_of_range&) {
  }
  return pos;
}

bool Decode::removeSof(TimingStream &data)
{
//  auto stream = prot->accessSection(0).getSoF().accessStream(); todo wegmachen...
//  if (stream.empty()) {
//    //no SoF
//    return true;
//  }
//  if (stream[0].first == false) {
//    //stream start with false => idle time before rx begins
//    stream.erase(stream.begin());
//  }
//
//  //SoF has bitwise coding of data -- search for exact match
//  try {
//    for (const auto& [state, time] : stream) {
//      auto &timing = data.timings().at(0);
//      if (state == true) {
//        if (checkTolerance(timing.mark_us, time)) {
//          //found item, remove
//          data.timings().erase(data.timings().begin());
//        } else {
//          return false;
//        }
//      } else {
//        if (checkTolerance(timing.pause_us, time)) {
//          //found item, remove
//          data.timings().erase(data.timings().begin());
//        } else {
//          return false;
//        }
//      }
//    }
//  } catch (out_of_range&) {
//    return false;
//  }
//  return true;
}

bool Decode::getPayload(int section, TimingStream &data, bool containsStartBit)
{
//  bool initialFalseChecked = false;
//
//  auto stream = prot->accessSection(section).getData().accessStream();
//  if (stream.empty()) {
//    //no SoF
//    return true;
//  }
//  if (stream[0].first == false) {
//    //stream start with false => idle time before rx begins
//    stream.erase(stream.begin());
//  }
//
//  try {
//    for (const auto& [state, time] : stream) {
//      auto &timing = data.timings().at(0);
//      if (state == true) {
//        if (checkTolerance(timing.mark_us, time)) {
//          //found item, remove
//          data.timings().erase(data.timings().begin());
//        } else {
//          return false;
//        }
//      } else {
//        if (checkTolerance(timing.pause_us, time)) {
//          //found item, remove
//          data.timings().erase(data.timings().begin());
//        } else {
//          return false;
//        }
//      }
//    }
//  } catch (out_of_range&) {
//    return false;
//  }
//  return true;

}

}
}

