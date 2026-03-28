/*
 * test_stream.cpp
 *
 * Unit tests for frame::Stream
 * Protocol reference: learn.md "Read stream"
 */

#include <gtest/gtest.h>
#include "stream.h"

using namespace frame;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// VALID_CHUNK_SIZE = 199
// Layout: [5 header][192 payload][2 terminator]
// header:     20 a3 01 02 70
// terminator: 01 30

static constexpr int kChunkSize    = 199;
static constexpr int kHeaderSize   = 5;
static constexpr int kTermSize     = 2;
static constexpr int kPayloadBytes = kChunkSize - kHeaderSize - kTermSize; // 192

static const std::vector<uint8_t> kHeader     = { 0x20, 0xA3, 0x01, 0x02, 0x70 };
static const std::vector<uint8_t> kTerminator = { 0x01, 0x30 };

// Build a well-formed 199-byte stream chunk with the given 192-byte payload.
static std::vector<uint8_t> makeValidChunk(const std::vector<uint8_t> &payload192)
{
    std::vector<uint8_t> chunk;
    chunk.insert(chunk.end(), kHeader.begin(), kHeader.end());
    chunk.insert(chunk.end(), payload192.begin(), payload192.end());
    chunk.insert(chunk.end(), kTerminator.begin(), kTerminator.end());
    return chunk;
}

// Build a 192-byte payload filled with repeated 16-bit big-endian words.
static std::vector<uint8_t> fillPayload(uint16_t word)
{
    std::vector<uint8_t> p;
    p.reserve(kPayloadBytes);
    for (int i = 0; i < kPayloadBytes / 2; ++i) {
        p.push_back(static_cast<uint8_t>(word >> 8));
        p.push_back(static_cast<uint8_t>(word & 0xFF));
    }
    return p;
}

// Real payload prefix from learn.md stream capture (first frame):
//   20 a3 01 02 70  03 72 06 f3 ...  01 30
// First two words: 0x0372=882, 0x06f3=1779
static std::vector<uint8_t> makeRealFirstWords()
{
    std::vector<uint8_t> p = fillPayload(0x0000);
    // overwrite first 4 bytes with real values
    p[0] = 0x03; p[1] = 0x72;  // 882
    p[2] = 0x06; p[3] = 0xf3;  // 1779
    return p;
}

// ---------------------------------------------------------------------------
// addChunk() – error path tests
// ---------------------------------------------------------------------------

TEST(StreamAddChunk, ErrSizeTooSmall)
{
    Stream s;
    std::vector<uint8_t> data = { 0x20, 0xA3, 0x01, 0x02 };  // 4 bytes < HEADER_MIN_SIZE=5
    EXPECT_EQ(s.addChunk(data), Stream::Status::ERR_SIZE);
}

TEST(StreamAddChunk, ErrSizeEmpty)
{
    Stream s;
    std::vector<uint8_t> data;
    EXPECT_EQ(s.addChunk(data), Stream::Status::ERR_SIZE);
}

TEST(StreamAddChunk, ErrFormatBadByte0)
{
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    chunk[0] = 0x00;
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_FORMAT);
}

TEST(StreamAddChunk, ErrFormatBadByte1)
{
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    chunk[1] = 0x00;
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_FORMAT);
}

TEST(StreamAddChunk, ErrFormatBadByte2)
{
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    chunk[2] = 0x00;
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_FORMAT);
}

TEST(StreamAddChunk, ErrFormatBadByte3)
{
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    chunk[3] = 0x00;
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_FORMAT);
}

TEST(StreamAddChunk, ErrFormatBadByte4)
{
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    chunk[4] = 0x00;
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_FORMAT);
}

TEST(StreamAddChunk, ErrTermBadLastByte)
{
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    chunk[kChunkSize - 1] = 0x00;  // corrupt last byte of terminator
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_TERM);
}

TEST(StreamAddChunk, ErrTermBadSecondLastByte)
{
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    chunk[kChunkSize - 2] = 0x00;  // corrupt first byte of terminator
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_TERM);
}

TEST(StreamAddChunk, ErrFormatWrongTotalSize)
{
    // Valid header and terminator but wrong total size
    Stream s;
    std::vector<uint8_t> chunk = kHeader;
    auto payload = fillPayload(0x0000);
    chunk.insert(chunk.end(), payload.begin(), payload.end() - 2);
    chunk.insert(chunk.end(), kTerminator.begin(), kTerminator.end());
    // size = 5 + 190 + 2 = 197, not 199
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::ERR_FORMAT);
}

// ---------------------------------------------------------------------------
// addChunk() – happy path
// ---------------------------------------------------------------------------

TEST(StreamAddChunk, OkSilencePayload)
{
    // Silence = "0000 8000" repeating (learn.md: "no mark, 32ms pause")
    Stream s;
    auto chunk = makeValidChunk(fillPayload(0x0000));
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::OK);
}

TEST(StreamAddChunk, OkRealFirstWords)
{
    Stream s;
    auto chunk = makeValidChunk(makeRealFirstWords());
    EXPECT_EQ(s.addChunk(chunk), Stream::Status::OK);
}

// ---------------------------------------------------------------------------
// getPayload() – parse verification
// Big-endian: v = p[i+1] | (p[i] << 8)
// ---------------------------------------------------------------------------

TEST(StreamPayload, EmptyBeforeAnyChunk)
{
    Stream s;
    EXPECT_TRUE(s.getPayload().empty());
}

TEST(StreamPayload, WordCountAfterOneChunk)
{
    // 192 payload bytes -> 96 words
    Stream s;
    s.addChunk(makeValidChunk(fillPayload(0x0372)));
    EXPECT_EQ(s.getPayload().size(), 96u);
}

TEST(StreamPayload, RealFirstTwoWords)
{
    // From learn.md: first two stream words are 882 (0x0372) and 1779 (0x06f3)
    Stream s;
    s.addChunk(makeValidChunk(makeRealFirstWords()));

    auto p = s.getPayload();
    ASSERT_GE(p.size(), 2u);
    EXPECT_EQ(p[0], 882u);
    EXPECT_EQ(p[1], 1779u);
}

TEST(StreamPayload, AllWordsFilledUniform)
{
    // Fill with known word, verify all 96 parse correctly
    Stream s;
    s.addChunk(makeValidChunk(fillPayload(0x06f3)));

    auto p = s.getPayload();
    ASSERT_EQ(p.size(), 96u);
    for (size_t i = 0; i < p.size(); ++i) {
        EXPECT_EQ(p[i], 0x06f3u) << "mismatch at word " << i;
    }
}

TEST(StreamPayload, SilenceWordValue)
{
    // Silence: 0x0000 -> word = 0
    Stream s;
    s.addChunk(makeValidChunk(fillPayload(0x0000)));

    auto p = s.getPayload();
    ASSERT_EQ(p.size(), 96u);
    EXPECT_EQ(p[0], 0u);
}

TEST(StreamPayload, MaxWordValue)
{
    // 0x8000 -> word = 32768 (used for silence marker in learn.md)
    Stream s;
    s.addChunk(makeValidChunk(fillPayload(0x8000)));

    auto p = s.getPayload();
    ASSERT_EQ(p.size(), 96u);
    EXPECT_EQ(p[0], 32768u);
}

TEST(StreamPayload, TwoChunksAccumulate)
{
    // Each chunk contributes 96 words, two chunks -> 192
    Stream s;
    s.addChunk(makeValidChunk(fillPayload(0x0372)));
    s.addChunk(makeValidChunk(fillPayload(0x06f3)));

    EXPECT_EQ(s.getPayload().size(), 192u);
}

TEST(StreamPayload, ClearPayloadResetsState)
{
    Stream s;
    s.addChunk(makeValidChunk(fillPayload(0x0372)));
    s.clearPayload();
    EXPECT_TRUE(s.getPayload().empty());
}

TEST(StreamPayload, ReuseAfterClear)
{
    Stream s;
    s.addChunk(makeValidChunk(fillPayload(0x0372)));
    s.clearPayload();
    s.addChunk(makeValidChunk(fillPayload(0x06f3)));

    auto p = s.getPayload();
    ASSERT_EQ(p.size(), 96u);
    EXPECT_EQ(p[0], 0x06f3u);
}

// ---------------------------------------------------------------------------
// get() – request frame
// ---------------------------------------------------------------------------

TEST(StreamGet, ReturnsFrameRequest)
{
    Stream s;
    auto req = s.get();
    ASSERT_EQ(req.size(), 4u);
    EXPECT_EQ(req[0], 0x20u);
    EXPECT_EQ(req[1], 0xA3u);
    EXPECT_EQ(req[2], 0x80u);
    EXPECT_EQ(req[3], 0x00u);
}
