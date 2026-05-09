/*
 * file.cpp
 *
 *  Created on: Apr 4, 2026
 *      Author: martin
 */

#include <fstream>

#include "file.h"

#include "lib/endian.h"

using namespace std;

namespace ssIr
{

void SerialStreamIr::setClock(double clock)
{
  if ((clock >= 31500) && (clock <= 250000)) {
    clockPeriod = 1.0 / clock * 1000000000.0;
  }
}

SerialStreamIr::SerialStreamIr()
{
}

SerialStreamIr::SerialStreamIr(const vector<uint16_t> &data, double clock)
{
  setClock(clock);

  stream = lib::TimingStream::fromMarkPause(data);
}

SerialStreamIr::SerialStreamIr(vector<uint16_t> data)
{
  uint16_t count;

  if (data.size() < HEADER_SIZE) {
    return;
  }

  if (data[0] > 0) {
    clockPeriod = data[0];
  }
  filler = data[1]; //normally 0
  if (filler != 0) {
    cout << "Warning: SsIr.bin filler not 0 (is " << filler << ")" << endl;
    //ignore, keep as-is
  }
  count = data[2];
  data.erase(data.begin(), data.begin() + HEADER_SIZE);
  if (data.size() != count) {
    cout << "Warning: SsIr.bin count invalid (is " << count << ", expected "
        << data.size() << ")" << endl;
    //ignore
  }

  //data is "mark" when MSB is set. needs to be alternating "mark" and "pause"
  for (int i = 0; i < data.size(); i += 2) {
    if ((data[i] & 0x8000) == 0) {
      cout << "Warning: SsIr.bin wordpos " << i << " adding \"mark\"" << endl;
      data.insert(data.begin() + i, 0);
    }
    data[i] = data[i] & 0x7fff;
  }
  stream = lib::TimingStream::fromMarkPause(data);
}

SerialStreamIr::SerialStreamIr(lib::TimingStream &stream, double clock)
{
  setClock(clock);

  this->stream = move(stream);
}

void SerialStreamIr::addData(vector<uint16_t> &data)
{
  stream.addMarkPause(data);
}

vector<uint16_t> SerialStreamIr::serialise() const
{
  vector<uint16_t> data;

  //header
  data.push_back(clockPeriod);
  data.push_back(filler);
  data.push_back(stream.timings().size() * 2); //1 mark/pause per item

  //stream data
  auto tmp = stream.convertMarkPause();
  for (int i = 0; i < tmp.size(); i++) {
    if ((tmp[i] & 0x8000) != 0) {
      //todo what should we do here? we could insert a block to add the missing time (most likely blank mark + remaining pause time). simple -> crop
      //crop
      tmp[i] = 0x7fff;
    }
    if ((i % 2) == 0) {
      tmp[i] = tmp[i] | 0x8000; //set mark
    }
  }
  data.insert(data.end(), tmp.begin(), tmp.end());

  return data;
}

File::File()
{
}

File::File(const string &filename)
{
  parse(filename);
}

Status File::parse(const string &filename)
{
  vector<uint16_t> offsetTable;

  streams.clear();

  ifstream file(filename, ios::binary);
  if (!file.is_open()) {
    return Status::ERROR_FILE;
  }

  auto raw = vector<uint8_t>(istreambuf_iterator<char>(file),
      istreambuf_iterator<char>());

  //minimum file size
  if (raw.size() < MIN_FILE_SIZE) {
    return Status::ERROR_SIZE;
  }

  //check header (equal)
  for (int i = 0; i < header.size(); i++) {
    if (header[i] != raw[i]) {
      cout << "Error: SsIr.bin header mismatch at " << i << " (is " << raw[i]
          << ", expected " << header[i] << ")" << endl;
      return Status::ERROR_FILE_FORMAT;
    }
  }
  raw.erase(raw.begin(), raw.begin() + HEADER_SIZE); //makes offset calculations easier

  //file size is word-aligned (+ first byte)
  if (((raw.size() + 1) % 2) != 0) {
    return Status::ERROR_SIZE;
  }

  auto objectCount = lib::parseHarmony16_file(raw[1], raw[2]);
  if (objectCount == 0) {
    //empty file
    return Status::OK;
  }

  auto ret = lib::parseHarmony16_file(
      { raw.begin() + 3, raw.begin() + 3 + 2 * objectCount }, offsetTable);
  if (ret != true) {
    return Status::ERROR_FILE_FORMAT;
  }
  //offsets need to be within the file
  for (const auto &o : offsetTable) {
    if (o > raw.size()) {
      return Status::ERROR_SIZE;
    }
  }

  for (int i = 0; i < objectCount; i++) {
    int start = offsetTable[i];
    int end = raw.size();
    if ((i + 1) < offsetTable.size()) {
      end = offsetTable[i + 1];
    }

    auto status = parseObject(raw, start, end);
    if (status != Status::OK) {
      return status;
    }
  }
  return Status::OK;
}

const SerialStreamIr& File::accessStream(int index) const
{
  return streams.at(index);
}

Status File::appendStream(vector<uint16_t> &data, double clock, int &index)
{
  index = streams.size();

  if ((data.size() % 2) != 0) {
    return Status::ERROR_SIZE;
  }

  auto s = SerialStreamIr(data, clock);
  streams.push_back(move(s));
  return Status::OK;
}

void File::appendStream(lib::TimingStream &stream, double clock, int &index)
{
  index = streams.size();

  auto s = SerialStreamIr(stream, clock);
  streams.push_back(move(s));
}

vector<uint8_t> File::serialise() const
{
  int pos;
  vector<uint8_t> raw;
  vector<vector<uint16_t>> dataStreams;

  raw.push_back(0); //fake first byte, fill later

  lib::setHarmony16_file(streams.size(), raw); //append object count

  pos = raw.size() + 2 * streams.size();
  for (const auto &stream : streams) {
    lib::setHarmony16_file(pos, raw); //append object start offset

    auto dataStream = stream.serialise();
    pos = pos + dataStream.size() * 2;

    dataStreams.push_back(dataStream);
  }

  //append data streams
  for (const auto &dataStream : dataStreams) {
    lib::setHarmony16_file(dataStream, raw);
  }

  //add header
  raw.erase(raw.begin()); //remove fake byte
  raw.insert(raw.begin(), header.begin(), header.end());
  return raw;
}

void File::removeStream(int index)
{
  if ((index < 0) || (index >= static_cast<int>(streams.size()))) {
    return;
  }
  streams.erase(streams.begin() + index);
}

Status File::serialise(const string &filename) const
{
  ofstream file(filename, ios::binary | ios::trunc); // binary + overwrite

  if (!file.is_open()) {
    return Status::ERROR_FILE;
  }

  auto raw = serialise();
  file.write(reinterpret_cast<const char*>(raw.data()), raw.size());

  if (!file.good()) {
    return Status::ERROR_FILE;
  }
  return Status::OK;
}

Status File::parseObject(const vector<uint8_t> &raw, uint16_t startOffset,
    uint16_t endOffset)
{
  vector<uint16_t> payload;
  auto ret = lib::parseHarmony16_file(
      { raw.begin() + startOffset, raw.begin() + endOffset }, payload);
  if (ret != true) {
    return Status::ERROR_SIZE;
  }

  auto stream = SerialStreamIr(payload);
  streams.push_back(move(stream));

  return Status::OK;
}

}

