// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for frame::Single
 * Protocol reference: learn.md "Read single frame"
 */

#include <gtest/gtest.h>
#include "trx_single.h"

using namespace trx;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Build a well-formed 18-byte chunk.
// eof_flag: 0x01 = more data, 0x00 = last chunk (DONE)
// payload12: exactly 12 bytes -> 4 triplets of { 0x02, HI, LO }
static std::vector<uint8_t> makeValidChunk(uint8_t eof_flag,
                                           const std::vector<uint8_t> &payload12)
{
    std::vector<uint8_t> chunk = { 0x20, 0xA2, 0x01, 0x05, 0x01, eof_flag };
    chunk.insert(chunk.end(), payload12.begin(), payload12.end());
    return chunk;
}

// Real first-chunk payload from learn.md capture:
//   20 a2 01 05 01 01 | 02 00 23  02 03 41  02 00 1e  02 06 f3
// Decoded words: 35, 833, 30, 1779
static const std::vector<uint8_t> kRealPayload1 = {
    0x02, 0x00, 0x23,   // word[0] = 35
    0x02, 0x03, 0x41,   // word[1] = 833
    0x02, 0x00, 0x1e,   // word[2] = 30
    0x02, 0x06, 0xf3    // word[3] = 1779
};

// Real second-chunk payload from learn.md:
//   20 a2 01 05 01 01 | 02 06 ea  02 0a 6b  02 03 71  02 0a 6a
// Decoded words: 1770, 2667, 881, 2666
static const std::vector<uint8_t> kRealPayload2 = {
    0x02, 0x06, 0xea,   // 1770
    0x02, 0x0a, 0x6b,   // 2667
    0x02, 0x03, 0x71,   // 881
    0x02, 0x0a, 0x6a    // 2666
};

// Real last-chunk payload (EoF) from learn.md:
//   20 a2 01 05 01 00 | 02 00 00  02 80 00  02 00 00  02 65 ae
// Decoded words: 0, 32768, 0, 26030
static const std::vector<uint8_t> kRealPayloadEof = {
    0x02, 0x00, 0x00,   // 0
    0x02, 0x80, 0x00,   // 32768
    0x02, 0x00, 0x00,   // 0
    0x02, 0x65, 0xae    // 26030
};

// ---------------------------------------------------------------------------
// addChunk() – error path tests
// ---------------------------------------------------------------------------

TEST(SingleAddChunk, ErrSizeTooSmall)
{
    Single s;
    std::vector<uint8_t> data = { 0x20, 0xA2, 0x01 };
    EXPECT_EQ(s.addChunk(data, true), Single::Status::ERR_SIZE);
}

TEST(SingleAddChunk, ErrSizeEmpty)
{
    Single s;
    std::vector<uint8_t> data;
    EXPECT_EQ(s.addChunk(data, true), Single::Status::ERR_SIZE);
}

TEST(SingleAddChunk, ErrResponseFormatBadByte0)
{
    Single s;
    auto chunk = makeValidChunk(0x01, kRealPayload1);
    chunk[0] = 0x00;
    EXPECT_EQ(s.addChunk(chunk, true), Single::Status::ERR_RESPONSE_FORMAT);
}

TEST(SingleAddChunk, ErrResponseFormatBadByte1)
{
    Single s;
    auto chunk = makeValidChunk(0x01, kRealPayload1);
    chunk[1] = 0x00;
    EXPECT_EQ(s.addChunk(chunk, true), Single::Status::ERR_RESPONSE_FORMAT);
}

TEST(SingleAddChunk, ErrTimeout)
{
    // data[2] == 0x02 -> timeout regardless of remaining bytes
    Single s;
    std::vector<uint8_t> data = { 0x20, 0xA2, 0x02, 0x05, 0x01 };
    EXPECT_EQ(s.addChunk(data, true), Single::Status::ERR_TIMEOUT);
}

TEST(SingleAddChunk, ErrReturnUnknownStatus)
{
    // data[2] != 0x01 and != 0x02 -> ERR_RETURN
    Single s;
    std::vector<uint8_t> data = { 0x20, 0xA2, 0x03, 0x05, 0x01 };
    EXPECT_EQ(s.addChunk(data, true), Single::Status::ERR_UNKNOWN_RETURNCODE);
}

TEST(SingleAddChunk, ErrResponseFormatWrongTotalSize)
{
    // data[2]=0x01 but total size != 18
    Single s;
    std::vector<uint8_t> data = { 0x20, 0xA2, 0x01, 0x05, 0x01, 0x01,
                                   0x02, 0x00, 0x23 };  // 9 bytes, not 18
    EXPECT_EQ(s.addChunk(data, true), Single::Status::ERR_RESPONSE_FORMAT);
}

TEST(SingleAddChunk, ErrResponseFormatBadData3)
{
    Single s;
    auto chunk = makeValidChunk(0x01, kRealPayload1);
    chunk[3] = 0x00;  // must be 0x05
    EXPECT_EQ(s.addChunk(chunk, true), Single::Status::ERR_RESPONSE_FORMAT);
}

TEST(SingleAddChunk, ErrResponseFormatBadData4)
{
    Single s;
    auto chunk = makeValidChunk(0x01, kRealPayload1);
    chunk[4] = 0x00;  // must be 0x01
    EXPECT_EQ(s.addChunk(chunk, true), Single::Status::ERR_RESPONSE_FORMAT);
}

TEST(SingleAddChunk, ErrPayloadFormatBadMarkerByte)
{
    Single s;
    std::vector<uint8_t> badPayload = kRealPayload1;
    badPayload[0] = 0x03;  // must be 0x02
    auto chunk = makeValidChunk(0x01, badPayload);
    EXPECT_EQ(s.addChunk(chunk, true), Single::Status::ERR_PAYLOAD_FORMAT);
}

// ---------------------------------------------------------------------------
// addChunk() – happy path
// ---------------------------------------------------------------------------

TEST(SingleAddChunk, OkMoreData)
{
    Single s;
    auto chunk = makeValidChunk(0x01, kRealPayload1);
    EXPECT_EQ(s.addChunk(chunk, true), Single::Status::OK);
}

TEST(SingleAddChunk, DoneOnEofFlag)
{
    Single s;
    auto chunk = makeValidChunk(0x00, kRealPayload1);
    EXPECT_EQ(s.addChunk(chunk, true), Single::Status::DONE);
}

TEST(SingleAddChunk, RealFirstChunk)
{
    // Verbatim from learn.md capture log
    Single s;
    std::vector<uint8_t> raw = { 0x20, 0xa2, 0x01, 0x05, 0x01, 0x01,
                                  0x02, 0x00, 0x23, 0x02, 0x03, 0x41,
                                  0x02, 0x00, 0x1e, 0x02, 0x06, 0xf3 };
    EXPECT_EQ(s.addChunk(raw, true), Single::Status::OK);
}

TEST(SingleAddChunk, RealLastChunk)
{
    // Verbatim EOF chunk from learn.md
    Single s;
    std::vector<uint8_t> raw = { 0x20, 0xa2, 0x01, 0x05, 0x01, 0x00,
                                  0x02, 0x00, 0x00, 0x02, 0x80, 0x00,
                                  0x02, 0x00, 0x00, 0x02, 0x65, 0xae };
    EXPECT_EQ(s.addChunk(raw, false), Single::Status::DONE);
}

// ---------------------------------------------------------------------------
// getPayload() and getExcess()
// Spec (single.h / learn.md):
//   raw payload words:   [w0,  w1,  w2,  w3,  w4, ...]
//   getExcess()     ->        [w0,  w2]        (unknown "???" bytes)
//   getPayload()  ->     [w1,  w3,  w4, ...] (timing values)
// ---------------------------------------------------------------------------

TEST(SinglePayload, EmptyBeforeAnyChunk)
{
    Single s;
    EXPECT_TRUE(s.getPayload().empty());
    EXPECT_TRUE(s.getExcess().empty());
}

TEST(SinglePayload, SingleChunkValues)
{
    // After one chunk with kRealPayload1 words: 35, 833, 30, 1779
    Single s;
    s.addChunk(makeValidChunk(0x01, kRealPayload1), true);

    auto p = s.getPayload();
    ASSERT_EQ(p.size(), 2u);
    EXPECT_EQ(p[0], 833u);   // w1
    EXPECT_EQ(p[1], 1779u);  // w3

    auto q = s.getExcess();
    ASSERT_EQ(q.size(), 2u);
    EXPECT_EQ(q[0], 35u);    // w0
    EXPECT_EQ(q[1], 30u);    // w2

    auto c = s.getClock();
    EXPECT_FLOAT_EQ(c, 36014.40576);
}

TEST(SinglePayload, TwoChunksAccumulate)
{
    // chunk1: [35, 833, 30, 1779]
    // chunk2: [1770, 2667, 881, 2666]
    // total raw:  [35, 833, 30, 1779, 1770, 2667, 881, 2666]
    // getPayload: [833, 1779, 1770, 2667, 881, 2666]
    // getExcess:       [35, 30]
    Single s;
    s.addChunk(makeValidChunk(0x01, kRealPayload1), true);
    s.addChunk(makeValidChunk(0x01, kRealPayload2), false);

    auto p = s.getPayload();
    ASSERT_EQ(p.size(), 6u);
    EXPECT_EQ(p[0], 833u);
    EXPECT_EQ(p[1], 1779u);
    EXPECT_EQ(p[2], 1770u);
    EXPECT_EQ(p[3], 2667u);
    EXPECT_EQ(p[4], 881u);
    EXPECT_EQ(p[5], 2666u);

    auto q = s.getExcess();
    ASSERT_EQ(q.size(), 2u);
    EXPECT_EQ(q[0], 35u);
    EXPECT_EQ(q[1], 30u);

    auto c = s.getClock();
    EXPECT_FLOAT_EQ(c, 36014.40576);
}

TEST(SinglePayload, EofChunkWithSilenceWords)
{
    // EOF chunk words: 0, 32768, 0, 26030
    // Combined with chunk1 raw: [35, 833, 30, 1779, 0, 32768, 0, 26030]
    // getPayload: [833, 1779, 0, 32768, 0, 26030]
    Single s;
    s.addChunk(makeValidChunk(0x01, kRealPayload1), true);
    s.addChunk(makeValidChunk(0x00, kRealPayloadEof), false);

    auto p = s.getPayload();
    ASSERT_EQ(p.size(), 6u);
    EXPECT_EQ(p[2], 0u);
    EXPECT_EQ(p[3], 32768u);
    EXPECT_EQ(p[4], 0u);
    EXPECT_EQ(p[5], 26030u);
}

// ---------------------------------------------------------------------------
// getError()
// ---------------------------------------------------------------------------

TEST(SingleGetError, ReturnsByte2)
{
    Single s;
    std::vector<uint8_t> data = { 0x20, 0xA2, 0x02, 0x05, 0x01 };
    EXPECT_EQ(s.getErrorByte(data), 0x02u);
}

TEST(SingleGetError, OkStatus)
{
    Single s;
    std::vector<uint8_t> data = { 0x20, 0xA2, 0x01, 0x05, 0x01 };
    EXPECT_EQ(s.getErrorByte(data), 0x01u);
}

TEST(SingleGetError, TooSmallReturns255)
{
    Single s;
    std::vector<uint8_t> data = { 0x20, 0xA2 };
    EXPECT_EQ(s.getErrorByte(data), 255u);
}

TEST(SingleGetError, EmptyReturns255)
{
    Single s;
    std::vector<uint8_t> data;
    EXPECT_EQ(s.getErrorByte(data), 255u);
}

// ---------------------------------------------------------------------------
// get() – request frame
// ---------------------------------------------------------------------------

TEST(SingleGet, ReturnsFrameRequest)
{
    Single s;
    auto req = s.get();
    ASSERT_EQ(req.size(), 4u);
    EXPECT_EQ(req[0], 0x20u);
    EXPECT_EQ(req[1], 0xA2u);
    EXPECT_EQ(req[2], 0x80u);
    EXPECT_EQ(req[3], 0x00u);
}
