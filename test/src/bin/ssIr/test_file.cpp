// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <filesystem>

#include "file.h"

using namespace ssIr;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Write a byte vector to a temp file, return the path.
static std::string writeTempFile(const std::vector<uint8_t> &data)
{
  auto path = std::filesystem::temp_directory_path() / "test_ssir_XXXXXX.bin";
  std::string pathStr = path.string();
  // use a fixed name per test run — unique enough for unit tests
  static int counter = 0;
  pathStr = (std::filesystem::temp_directory_path()
      / ("test_ssir_" + std::to_string(counter++) + ".bin")).string();

  std::ofstream f(pathStr, std::ios::binary | std::ios::trunc);
  f.write(reinterpret_cast<const char*>(data.data()), data.size());
  return pathStr;
}

static void removeTempFile(const std::string &path)
{
  std::filesystem::remove(path);
}

// Minimal valid mark/pause data: two pairs from learn.md
static std::vector<uint16_t> simpleMarkPause()
{
  return {833, 946, 1770, 897};
}

// ===========================================================================
// SerialStreamIr – default constructor
// ===========================================================================

TEST(SerialStreamIrDefault, DefaultClockIs38kHz)
{
  SerialStreamIr s;
  // default clockPeriod = 26316 ns -> 1e9/26316 ≈ 38000 Hz
  EXPECT_NEAR(s.getClock(), 38000.0, 100.0);
}

TEST(SerialStreamIrDefault, EmptyStream)
{
  SerialStreamIr s;
  EXPECT_TRUE(s.accessStream().timings().empty());
}

// ===========================================================================
// SerialStreamIr – constructor(data, clock)
// ===========================================================================

TEST(SerialStreamIrDataClock, ClockStoredAndRetrieved)
{
  SerialStreamIr s(simpleMarkPause(), 38000.0);
  EXPECT_NEAR(s.getClock(), 38000.0, 200.0);
}

TEST(SerialStreamIrDataClock, ClockBelowRangeIgnored)
{
  SerialStreamIr s(simpleMarkPause(), 100.0); // below 5000 Hz
  // setClock should reject it, clockPeriod stays at default 26316
  EXPECT_NEAR(s.getClock(), 38000.0, 100.0);
}

TEST(SerialStreamIrDataClock, ClockAboveRangeIgnored)
{
  SerialStreamIr s(simpleMarkPause(), 999999.0); // above 250000 Hz
  EXPECT_NEAR(s.getClock(), 38000.0, 100.0);
}

TEST(SerialStreamIrDataClock, ClockAtLowerBound)
{
  SerialStreamIr s(simpleMarkPause(), 31500.0);
  EXPECT_NEAR(s.getClock(), 31500.0, 50.0);
}

TEST(SerialStreamIrDataClock, ClockAtUpperBound)
{
  SerialStreamIr s(simpleMarkPause(), 250000.0);
  EXPECT_NEAR(s.getClock(), 250000.0, 500.0);
}

TEST(SerialStreamIrDataClock, StreamDataStored)
{
  SerialStreamIr s(simpleMarkPause(), 38000.0);
  const auto &t = s.accessStream().timings();
  ASSERT_EQ(t.size(), 2u);
  EXPECT_EQ(t[0].mark_us, 833u);
  EXPECT_EQ(t[0].pause_us, 946u);
  EXPECT_EQ(t[1].mark_us, 1770u);
  EXPECT_EQ(t[1].pause_us, 897u);
}

TEST(SerialStreamIrDataClock, EmptyData)
{
  SerialStreamIr s( { }, 38000.0);
  EXPECT_TRUE(s.accessStream().timings().empty());
}

// ===========================================================================
// SerialStreamIr – constructor(TimingStream, clock)
// ===========================================================================

TEST(SerialStreamIrTimingStream, TakesOwnership)
{
  auto ts = lib::TimingStream::fromMarkPause(simpleMarkPause());
  SerialStreamIr s(ts, 38000.0);
  ASSERT_EQ(s.accessStream().timings().size(), 2u);
  EXPECT_EQ(s.accessStream().timings()[0].mark_us, 833u);
}

// ===========================================================================
// SerialStreamIr – constructor(vector<uint16_t>) — raw file format
// Header: [clockPeriod, filler, count, mark0, pause0, mark1, pause1, ...]
// MSB set = mark. Values are stripped of MSB before storage.
// ===========================================================================

// Build a raw payload vector as the file format uses it.
// clock_ns: clock period in ns (stored as-is in word[0])
// pairs: alternating mark/pause in µs (mark will have MSB set)
static std::vector<uint16_t> buildRawPayload(uint16_t clock_ns,
    const std::vector<uint16_t> &markPause)
{
  std::vector<uint16_t> raw;
  raw.push_back(clock_ns);
  raw.push_back(0); // filler
  raw.push_back(static_cast<uint16_t>(markPause.size())); // count

  for (size_t i = 0; i < markPause.size(); i++) {
    if ((i % 2) == 0) {
      // mark: set MSB
      raw.push_back(markPause[i] | 0x8000);
    } else {
      // pause: no MSB
      raw.push_back(markPause[i]);
    }
  }
  return raw;
}

TEST(SerialStreamIrRaw, ParsesClockPeriod)
{
  auto raw = buildRawPayload(26316, simpleMarkPause());
  SerialStreamIr s(raw);
  EXPECT_NEAR(s.getClock(), 38000.0, 100.0);
}

TEST(SerialStreamIrRaw, ParsesMarkPauseData)
{
  auto raw = buildRawPayload(26316, simpleMarkPause());
  SerialStreamIr s(raw);
  const auto &t = s.accessStream().timings();
  ASSERT_EQ(t.size(), 2u);
  EXPECT_EQ(t[0].mark_us, 833u);
  EXPECT_EQ(t[0].pause_us, 946u);
  EXPECT_EQ(t[1].mark_us, 1770u);
  EXPECT_EQ(t[1].pause_us, 897u);
}

TEST(SerialStreamIrRaw, TooSmallReturnsEmpty)
{
  // fewer than HEADER_SIZE=3 words -> constructor returns early
  std::vector<uint16_t> raw = { 26316, 0 }; // only 2 words
  SerialStreamIr s(raw);
  EXPECT_TRUE(s.accessStream().timings().empty());
}

TEST(SerialStreamIrRaw, ZeroClockKeepsDefault)
{
  // clock word == 0 -> the `if (data[0] > 0)` check rejects it
  auto raw = buildRawPayload(0, simpleMarkPause());
  SerialStreamIr s(raw);
  EXPECT_NEAR(s.getClock(), 38000.0, 100.0); // default unchanged
}

TEST(SerialStreamIrRaw, MissingMarkMsbInsertsSilentMark)
{
  // If a word at an even position has no MSB set, a 0-mark is inserted.
  // Build payload where mark has no MSB (simulates the repair path).
  std::vector<uint16_t> raw;
  raw.push_back(26316); // clock
  raw.push_back(0);     // filler
  raw.push_back(2);     // count = 2 words
  raw.push_back(833);   // mark WITHOUT MSB set — triggers repair
  raw.push_back(946);   // pause
  SerialStreamIr s(raw);
  // After inserting a 0-mark before pause, data becomes [0, 833, 0, 946]
  const auto &t = s.accessStream().timings();
  ASSERT_EQ(t.size(), 2u);
  EXPECT_EQ(t[0].mark_us, 0u);
  EXPECT_EQ(t[0].pause_us, 833u);
  EXPECT_EQ(t[1].mark_us, 0u);
  EXPECT_EQ(t[1].pause_us, 946u);

  //todo is this the correct way to handle this -- or should we error-out?
}

// ===========================================================================
// SerialStreamIr – addData
// ===========================================================================

TEST(SerialStreamIrAddData, AppendsToStream)
{
  SerialStreamIr s(simpleMarkPause(), 38000.0);
  std::vector<uint16_t> more = { 881, 946 };
  s.addData(more);
  EXPECT_EQ(s.accessStream().timings().size(), 3u);
}

// ===========================================================================
// SerialStreamIr – serialise / round-trip
// ===========================================================================

TEST(SerialStreamIrSerialise, HeaderWords)
{
  SerialStreamIr s(simpleMarkPause(), 38000.0);
  auto out = s.serialise();

  // word[0]: clockPeriod (stored as uint16_t, truncated from double)
  // word[1]: filler = 0
  // word[2]: count = timings * 2 = 4
  ASSERT_GE(out.size(), 3u);
  EXPECT_EQ(out[1], 0u);   // filler
  EXPECT_EQ(out[2], 4u);   // 2 pairs * 2
}

TEST(SerialStreamIrSerialise, PayloadWords)
{
  SerialStreamIr s(simpleMarkPause(), 38000.0);
  auto out = s.serialise();

  // words [3..6]: mark0, pause0, mark1, pause1
  ASSERT_EQ(out.size(), 7u);
  EXPECT_EQ(out[3], 0x8000 | 833u);
  EXPECT_EQ(out[4], 946u);
  EXPECT_EQ(out[5], 0x8000 | 1770u);
  EXPECT_EQ(out[6], 897u);
}

TEST(SerialStreamIrSerialise, EmptyStreamSerialises)
{
  SerialStreamIr s;
  auto out = s.serialise();
  ASSERT_EQ(out.size(), 3u); // only header words, count=0
  EXPECT_EQ(out[2], 0u);
}

TEST(SerialStreamIrSerialise, RoundTripViaRawConstructor)
{
  SerialStreamIr s(simpleMarkPause(), 38000.0);
  auto serialised = s.serialise();

  SerialStreamIr s2(serialised);
  const auto &t = s2.accessStream().timings();
  ASSERT_EQ(t.size(), 2u);
  EXPECT_EQ(t[0].mark_us, 833u);
  EXPECT_EQ(t[0].pause_us, 946u);
  EXPECT_EQ(t[1].mark_us, 1770u);
  EXPECT_EQ(t[1].pause_us, 897u);
}

// ===========================================================================
// File – empty file
// ===========================================================================

TEST(FileEmpty, DefaultIsEmpty)
{
  File f;
  EXPECT_TRUE(f.isEmpty());
  EXPECT_EQ(f.getStreamCount(), 0);
}

// ===========================================================================
// File – parse errors
// ===========================================================================

TEST(FileParse, NonExistentFile)
{
  File f;
  auto status = f.parse("/tmp/does_not_exist_ssir_test.bin");
  EXPECT_EQ(status, Status::ERROR_FILE);
}

TEST(FileParse, TooSmall)
{
  auto path = writeTempFile( { 0x01, 0x01, 0x05 }); // 3 bytes < MIN_FILE_SIZE=8
  File f;
  auto status = f.parse(path);
  EXPECT_EQ(status, Status::ERROR_SIZE);
  removeTempFile(path);
}

TEST(FileParse, WrongHeader)
{
  // Valid size but wrong header byte
  std::vector<uint8_t> data = { 0xFF, 0x01, 0x05, 0x00, 0x00, 0x01, 0x00, 0x00 };
  auto path = writeTempFile(data);
  File f;
  auto status = f.parse(path);
  EXPECT_EQ(status, Status::ERROR_FILE_FORMAT);
  removeTempFile(path);
}

TEST(FileParse, ValidHeaderEmptyCommandCount)
{
  // Header + 0x01 byte + count=0 (little-endian word 0x0000)
  // raw after stripping 5-byte header: [0x01, 0x00, 0x00]
  // objectCount = parseHarmony16_file(raw[1], raw[2]) = 0 -> OK, empty
  std::vector<uint8_t> data = { 0x01, 0x01, 0x05, 0x00, 0x00, // header (5 bytes stripped by parse)
      0x01,                          // unknown byte (always 1)
      0x00, 0x00                     // count = 0
      };
  auto path = writeTempFile(data);
  File f;
  auto status = f.parse(path);
  EXPECT_EQ(status, Status::OK);
  EXPECT_TRUE(f.isEmpty());
  removeTempFile(path);
}

// ===========================================================================
// File – appendStream + getStreamCount
// ===========================================================================

TEST(FileAppend, AppendVectorIncrementsCount)
{
  File f;
  auto data = simpleMarkPause();
  int index = -1;
  auto status = f.appendStream(data, 38000.0, index);
  EXPECT_EQ(status, Status::OK);
  EXPECT_EQ(f.getStreamCount(), 1);
  EXPECT_EQ(index, 0);
}

TEST(FileAppend, AppendVectorOddSizeRejected)
{
  File f;
  std::vector<uint16_t> data = { 833, 946, 1770 }; // odd count
  int index = -1;
  auto status = f.appendStream(data, 38000.0, index);
  EXPECT_EQ(status, Status::ERROR_SIZE);
  EXPECT_EQ(f.getStreamCount(), 0);
}

TEST(FileAppend, AppendTimingStream)
{
  File f;
  auto ts = lib::TimingStream::fromMarkPause(simpleMarkPause());
  int index = -1;
  f.appendStream(ts, 38000.0, index);
  EXPECT_EQ(f.getStreamCount(), 1);
  EXPECT_EQ(index, 0);
}

TEST(FileAppend, AppendTwoStreams)
{
  File f;
  auto data = simpleMarkPause();
  int idx0 = -1, idx1 = -1;
  f.appendStream(data, 38000.0, idx0);
  f.appendStream(data, 38000.0, idx1);
  EXPECT_EQ(f.getStreamCount(), 2);
  EXPECT_EQ(idx0, 0);
  EXPECT_EQ(idx1, 1);
}

TEST(FileAppend, IndexReflectsPosition)
{
  File f;
  auto data = simpleMarkPause();
  int index = -1;
  f.appendStream(data, 38000.0, index);
  f.appendStream(data, 38000.0, index);
  f.appendStream(data, 38000.0, index);
  EXPECT_EQ(index, 2);
}

// ---------------------------------------------------------------------------
// File – removeStream
// ---------------------------------------------------------------------------

TEST(FileRemoveStream, RemovesOnlyStream)
{
    File f;
    auto data = simpleMarkPause();
    int index = -1;
    f.appendStream(data, 38000.0, index);

    f.removeStream(0);
    EXPECT_EQ(f.getStreamCount(), 0);
    EXPECT_TRUE(f.isEmpty());
}

TEST(FileRemoveStream, RemovesFirstOfTwo)
{
    File f;
    auto data = simpleMarkPause();
    std::vector<uint16_t> data2 = { 881, 946 };
    int idx = -1;
    f.appendStream(data,  38000.0, idx);
    f.appendStream(data2, 36000.0, idx);

    f.removeStream(0);

    ASSERT_EQ(f.getStreamCount(), 1);
    EXPECT_NEAR(f.accessStream(0).getClock(), 36000.0, 200.0);
}

TEST(FileRemoveStream, RemovesLastOfTwo)
{
    File f;
    auto data = simpleMarkPause();
    std::vector<uint16_t> data2 = { 881, 946 };
    int idx = -1;
    f.appendStream(data,  38000.0, idx);
    f.appendStream(data2, 36000.0, idx);

    f.removeStream(1);

    ASSERT_EQ(f.getStreamCount(), 1);
    EXPECT_NEAR(f.accessStream(0).getClock(), 38000.0, 200.0);
}

TEST(FileRemoveStream, RemovesMiddleOfThree)
{
    File f;
    auto data = simpleMarkPause();
    int idx = -1;
    f.appendStream(data, 38000.0, idx);
    f.appendStream(data, 36000.0, idx);
    f.appendStream(data, 40000.0, idx);

    f.removeStream(1);

    ASSERT_EQ(f.getStreamCount(), 2);
    EXPECT_NEAR(f.accessStream(0).getClock(), 38000.0, 200.0);
    EXPECT_NEAR(f.accessStream(1).getClock(), 40000.0, 200.0);
}

TEST(FileRemoveStream, RemoveFromEmptyIsNoOp)
{
    File f;
    f.removeStream(0);
    EXPECT_EQ(f.getStreamCount(), 0);
}

TEST(FileRemoveStream, NegativeIndexIsNoOp)
{
    File f;
    auto data = simpleMarkPause();
    int idx = -1;
    f.appendStream(data, 38000.0, idx);

    f.removeStream(-1);
    EXPECT_EQ(f.getStreamCount(), 1);
}

TEST(FileRemoveStream, IndexEqualToSizeIsNoOp)
{
    File f;
    auto data = simpleMarkPause();
    int idx = -1;
    f.appendStream(data, 38000.0, idx);

    f.removeStream(1); // size is 1, valid indices are 0..0
    EXPECT_EQ(f.getStreamCount(), 1);
}

TEST(FileRemoveStream, IndexWayOutOfRangeIsNoOp)
{
    File f;
    auto data = simpleMarkPause();
    int idx = -1;
    f.appendStream(data, 38000.0, idx);

    f.removeStream(999);
    EXPECT_EQ(f.getStreamCount(), 1);
}

// ===========================================================================
// File – accessStream
// ===========================================================================

TEST(FileAccess, AccessStreamReturnsCorrectData)
{
  File f;
  auto data = simpleMarkPause();
  int index = -1;
  f.appendStream(data, 38000.0, index);

  const auto &s = f.accessStream(0);
  const auto &t = s.accessStream().timings();
  ASSERT_EQ(t.size(), 2u);
  EXPECT_EQ(t[0].mark_us, 833u);
}

TEST(FileAccess, OutOfRangeThrows)
{
  File f;
  EXPECT_THROW(f.accessStream(0), std::out_of_range);
}

// ===========================================================================
// File – serialise (in-memory) round-trip
//
// NOTE: Due to BUG-3 (offset table off-by-one in serialise()), parse() of
// serialised output is expected to FAIL or produce garbled data. These tests
// document the current behaviour. When the bug is fixed, the EXPECT_NE on
// status should become EXPECT_EQ(status, Status::OK).
// ===========================================================================

TEST(FileSerialise, OutputStartsWithHeader)
{
  File f;
  auto data = simpleMarkPause();
  int index = -1;
  f.appendStream(data, 38000.0, index);

  auto raw = f.serialise();
  ASSERT_GE(raw.size(), 6u);
  EXPECT_EQ(raw[0], 0x01u);
  EXPECT_EQ(raw[1], 0x01u);
  EXPECT_EQ(raw[2], 0x05u);
  EXPECT_EQ(raw[3], 0x00u);
  EXPECT_EQ(raw[4], 0x00u);
  EXPECT_EQ(raw[5], 0x01u);
}

TEST(FileSerialise, EmptyFileSerialises)
{
  File f;
  auto raw = f.serialise();
  // header (6) + count word (2) = 8 bytes minimum
  EXPECT_GE(raw.size(), 8u);
  EXPECT_EQ(raw[0], 0x01u);
}

TEST(FileSerialise, NonEmptyProducesLargerOutput)
{
  File empty;
  auto emptyRaw = empty.serialise();

  File f;
  auto data = simpleMarkPause();
  int index = -1;
  f.appendStream(data, 38000.0, index);
  auto fullRaw = f.serialise();

  EXPECT_GT(fullRaw.size(), emptyRaw.size());
}

TEST(FileSerialise, WriteAndReadFile)
{
  File f;
  auto data = simpleMarkPause();
  int index = -1;
  f.appendStream(data, 38000.0, index);

  auto path = std::filesystem::temp_directory_path() / "test_ssir_write.bin";
  auto status = f.serialise(path.string());
  EXPECT_EQ(status, Status::OK);

  // File must exist and be non-empty
  EXPECT_TRUE(std::filesystem::exists(path));
  EXPECT_GT(std::filesystem::file_size(path), 0u);

  std::filesystem::remove(path);
}

TEST(FileSerialise, WriteToInvalidPath)
{
  File f;
  auto status = f.serialise("/nonexistent_dir/test.bin");
  EXPECT_EQ(status, Status::ERROR_FILE);
}

// ===========================================================================
// Real SsIr.bin fixture
//
// File structure (verified by hex inspection):
//   Header:    01 01 05 00 00 01
//   Count:     2 objects
//   Offsets:   0x0007 (object 0), 0x030d (object 1)
//
//   Object 0:  clockPeriod=27779 ns (~36 kHz), filler=0, count=384 words
//              192 mark/pause pairs
//              first pair: mark=2639 us, pause=909 us
//              last pair:  mark=438 us,  pause=32330 us (inter-frame silence)
//
//   Object 1:  clockPeriod=26500 ns (~37.7 kHz), filler=0, count=38 words
//              19 mark/pause pairs
//              first pair: mark=53 us,  pause=1719 us
//              last pair:  mark=879 us, pause=31889 us (inter-frame silence)
//
// REAL_SSIR_BIN_PATH is injected via cmake:
//   target_compile_definitions(test_ssir PRIVATE
//       REAL_SSIR_BIN_PATH="${CMAKE_CURRENT_SOURCE_DIR}/SsIr.bin")
// ===========================================================================

#ifndef REAL_SSIR_BIN_PATH
#define REAL_SSIR_BIN_PATH "SsIr.bin"
#endif

class RealFileTest: public ::testing::Test
{
  protected:
    File file;
    std::string binPath = REAL_SSIR_BIN_PATH;

    void SetUp() override
    {
      auto status = file.parse(binPath);
      ASSERT_EQ(status, Status::OK)<< "Could not parse " << binPath;
    }
  };

// ---------------------------------------------------------------------------
// Read: top-level structure
// ---------------------------------------------------------------------------

TEST_F(RealFileTest, NotEmpty)
{
  EXPECT_FALSE(file.isEmpty());
}

TEST_F(RealFileTest, StreamCount)
{
  EXPECT_EQ(file.getStreamCount(), 2);
}

// ---------------------------------------------------------------------------
// Read: object 0
// ---------------------------------------------------------------------------

TEST_F(RealFileTest, Object0ClockFrequency)
{
  // clockPeriod=27779 ns -> ~35998 Hz
  EXPECT_NEAR(file.accessStream(0).getClock(), 36000.0, 200.0);
}

TEST_F(RealFileTest, Object0PairCount)
{
  EXPECT_EQ(file.accessStream(0).accessStream().timings().size(), 192u);
}

TEST_F(RealFileTest, Object0FirstPair)
{
  const auto &t = file.accessStream(0).accessStream().timings();
  ASSERT_GE(t.size(), 1u);
  EXPECT_EQ(t[0].mark_us, 2639u);
  EXPECT_EQ(t[0].pause_us, 909u);
}

TEST_F(RealFileTest, Object0LastPair)
{
  const auto &t = file.accessStream(0).accessStream().timings();
  ASSERT_GE(t.size(), 192u);
  // last pair is the inter-frame silence
  EXPECT_EQ(t[191].mark_us, 438u);
  EXPECT_EQ(t[191].pause_us, 32330u);
}

// ---------------------------------------------------------------------------
// Read: object 1
// ---------------------------------------------------------------------------

TEST_F(RealFileTest, Object1ClockFrequency)
{
  // clockPeriod=26500 ns -> ~37735 Hz
  EXPECT_NEAR(file.accessStream(1).getClock(), 37736.0, 200.0);
}

TEST_F(RealFileTest, Object1PairCount)
{
  EXPECT_EQ(file.accessStream(1).accessStream().timings().size(), 19u);
}

TEST_F(RealFileTest, Object1FirstPair)
{
  const auto &t = file.accessStream(1).accessStream().timings();
  ASSERT_GE(t.size(), 1u);
  EXPECT_EQ(t[0].mark_us, 53u);
  EXPECT_EQ(t[0].pause_us, 1719u);
}

TEST_F(RealFileTest, Object1LastPair)
{
  const auto &t = file.accessStream(1).accessStream().timings();
  ASSERT_GE(t.size(), 19u);
  EXPECT_EQ(t[18].mark_us, 879u);
  EXPECT_EQ(t[18].pause_us, 31889u);
}

TEST_F(RealFileTest, OutOfRangeAccessThrows)
{
  EXPECT_THROW(file.accessStream(2), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Write: serialise and binary compare (round-trip)
// ---------------------------------------------------------------------------

TEST_F(RealFileTest, SerialisedBinaryMatchesOriginal)
{
  // Read original file bytes
  std::ifstream orig(binPath, std::ios::binary);
  ASSERT_TRUE(orig.is_open());
  std::vector<uint8_t> expected( { std::istreambuf_iterator<char>(orig),
      std::istreambuf_iterator<char>() });

  // Serialise and compare
  auto actual = file.serialise();

  ASSERT_EQ(actual.size(), expected.size())<< "Serialised size " << actual.size()
  << " != original size " << expected.size();

  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(actual[i], expected[i]) << "Byte mismatch at offset " << i
        << " (got 0x" << std::hex << static_cast<int>(actual[i])
        << ", expected 0x" << static_cast<int>(expected[i]) << std::dec << ")";
    if (actual[i] != expected[i]) {
      break; // stop at first diff to keep output readable
    }
  }
}

TEST_F(RealFileTest, WriteFileBinaryMatchesOriginal)
{
  auto outPath =
      (std::filesystem::temp_directory_path() / "SsIr_roundtrip.bin").string();

  auto status = file.serialise(outPath);
  ASSERT_EQ(status, Status::OK);

  // Read original
  std::ifstream orig(binPath, std::ios::binary);
  ASSERT_TRUE(orig.is_open());
  std::vector<uint8_t> expected( { std::istreambuf_iterator<char>(orig),
      std::istreambuf_iterator<char>() });

  // Read written file
  std::ifstream written(outPath, std::ios::binary);
  ASSERT_TRUE(written.is_open());
  std::vector<uint8_t> actual( { std::istreambuf_iterator<char>(written),
      std::istreambuf_iterator<char>() });

  ASSERT_EQ(actual.size(), expected.size())<< "Written file size " << actual.size()
  << " != original size " << expected.size();

  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(actual[i], expected[i]) << "Byte mismatch at offset " << i
        << " (got 0x" << std::hex << static_cast<int>(actual[i])
        << ", expected 0x" << static_cast<int>(expected[i]) << std::dec << ")";
    if (actual[i] != expected[i]) {
      break;
    }
  }

  std::filesystem::remove(outPath);
}
