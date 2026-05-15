// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Unit tests for lib::Block and lib::TimingStream
 * Skips visual/string converters (gnuplot, hex, int, ascii).
 */

#include <gtest/gtest.h>
#include "timing.h"

using namespace binary;

// ---------------------------------------------------------------------------
// Block::fromMarkSegment
// segment_us = mark_us + pause_us  ->  pause_us = segment_us - mark_us
// ---------------------------------------------------------------------------

TEST(BlockFromMarkSegment, Values)
{
    auto b = Block::fromMarkSegment(833, 1779);
    EXPECT_EQ(b.mark_us,    833u);
    EXPECT_EQ(b.segment_us, 1779u);
    EXPECT_EQ(b.pause_us,   946u);   // 1779 - 833
}

TEST(BlockFromMarkSegment, ZeroMark)
{
    auto b = Block::fromMarkSegment(0, 1000);
    EXPECT_EQ(b.mark_us,    0u);
    EXPECT_EQ(b.segment_us, 1000u);
    EXPECT_EQ(b.pause_us,   1000u);
}

TEST(BlockFromMarkSegment, EqualMarkAndSegment)
{
    // pause = 0
    auto b = Block::fromMarkSegment(500, 500);
    EXPECT_EQ(b.mark_us,    500u);
    EXPECT_EQ(b.segment_us, 500u);
    EXPECT_EQ(b.pause_us,   0u);
}

TEST(BlockFromMarkSegment, ChronoAccessors)
{
    auto b = Block::fromMarkSegment(833, 1779);
    EXPECT_EQ(b.mark().count(),    833);
    EXPECT_EQ(b.segment().count(), 1779);
    EXPECT_EQ(b.pause().count(),   946);
}

// ---------------------------------------------------------------------------
// Block::fromMarkPause
// segment_us = mark_us + pause_us
// ---------------------------------------------------------------------------

TEST(BlockFromMarkPause, Values)
{
    auto b = Block::fromMarkPause(833, 946);
    EXPECT_EQ(b.mark_us,    833u);
    EXPECT_EQ(b.pause_us,   946u);
    EXPECT_EQ(b.segment_us, 1779u);  // 833 + 946
}

TEST(BlockFromMarkPause, ZeroMark)
{
    auto b = Block::fromMarkPause(0, 1000);
    EXPECT_EQ(b.mark_us,    0u);
    EXPECT_EQ(b.pause_us,   1000u);
    EXPECT_EQ(b.segment_us, 1000u);
}

TEST(BlockFromMarkPause, ZeroPause)
{
    auto b = Block::fromMarkPause(500, 0);
    EXPECT_EQ(b.mark_us,    500u);
    EXPECT_EQ(b.pause_us,   0u);
    EXPECT_EQ(b.segment_us, 500u);
}

TEST(BlockFromMarkPause, ChronoAccessors)
{
    auto b = Block::fromMarkPause(833, 946);
    EXPECT_EQ(b.mark().count(),    833);
    EXPECT_EQ(b.pause().count(),   946);
    EXPECT_EQ(b.segment().count(), 1779);
}

// ---------------------------------------------------------------------------
// Block round-trip: fromMarkSegment -> fromMarkPause should give same values
// Using real timing values from learn.md:
//   mark=833, segment=1779  ->  pause=946
// ---------------------------------------------------------------------------

TEST(BlockRoundTrip, MarkSegmentToMarkPause)
{
    auto a = Block::fromMarkSegment(833, 1779);
    auto b = Block::fromMarkPause(a.mark_us, a.pause_us);

    EXPECT_EQ(a.mark_us,    b.mark_us);
    EXPECT_EQ(a.pause_us,   b.pause_us);
    EXPECT_EQ(a.segment_us, b.segment_us);
}

// ---------------------------------------------------------------------------
// TimingStream::addMarkSegment / fromMarkSegment
// Raw input pairs: [mark, segment, mark, segment, ...]
// Learn.md single frame decoded: 833,1779  1770,2667  881,2666 ...
// ---------------------------------------------------------------------------

TEST(TimingStreamMarkSegment, EmptyInput)
{
    auto ts = TimingStream::fromMarkSegment({});
    EXPECT_TRUE(ts.timings().empty());
}

TEST(TimingStreamMarkSegment, OddLengthIgnoresLastElement)
{
    // odd number of words: last one has no pair, must be ignored
    auto ts = TimingStream::fromMarkSegment({ 833, 1779, 881 });
    ASSERT_EQ(ts.timings().size(), 1u);
}

TEST(TimingStreamMarkSegment, SinglePair)
{
    auto ts = TimingStream::fromMarkSegment({ 833, 1779 });
    ASSERT_EQ(ts.timings().size(), 1u);
    EXPECT_EQ(ts.timings()[0].mark_us,    833u);
    EXPECT_EQ(ts.timings()[0].segment_us, 1779u);
    EXPECT_EQ(ts.timings()[0].pause_us,   946u);
}

TEST(TimingStreamMarkSegment, RealLearnMdValues)
{
    // First four decoded words from learn.md single frame capture:
    // 833,1779  1770,2667
    std::vector<uint16_t> raw = { 833, 1779, 1770, 2667 };
    auto ts = TimingStream::fromMarkSegment(raw);

    ASSERT_EQ(ts.timings().size(), 2u);

    EXPECT_EQ(ts.timings()[0].mark_us,  833u);
    EXPECT_EQ(ts.timings()[0].pause_us, 946u);   // 1779-833

    EXPECT_EQ(ts.timings()[1].mark_us,  1770u);
    EXPECT_EQ(ts.timings()[1].pause_us, 897u);   // 2667-1770
}

TEST(TimingStreamMarkSegment, SilenceBlock)
{
    // From learn.md: silence = mark=0, segment=32768
    std::vector<uint16_t> raw = { 0, 32768 };
    auto ts = TimingStream::fromMarkSegment(raw);

    ASSERT_EQ(ts.timings().size(), 1u);
    EXPECT_EQ(ts.timings()[0].mark_us,    0u);
    EXPECT_EQ(ts.timings()[0].segment_us, 32768u);
    EXPECT_EQ(ts.timings()[0].pause_us,   32768u);
}

// ---------------------------------------------------------------------------
// TimingStream::addMarkPause / fromMarkPause
// Raw input pairs: [mark, pause, mark, pause, ...]
// ---------------------------------------------------------------------------

TEST(TimingStreamMarkPause, EmptyInput)
{
    auto ts = TimingStream::fromMarkPause({});
    EXPECT_TRUE(ts.timings().empty());
}

TEST(TimingStreamMarkPause, OddLengthIgnoresLastElement)
{
    auto ts = TimingStream::fromMarkPause({ 833, 946, 881 });
    ASSERT_EQ(ts.timings().size(), 1u);
}

TEST(TimingStreamMarkPause, SinglePair)
{
    auto ts = TimingStream::fromMarkPause({ 833, 946 });
    ASSERT_EQ(ts.timings().size(), 1u);
    EXPECT_EQ(ts.timings()[0].mark_us,  833u);
    EXPECT_EQ(ts.timings()[0].pause_us, 946u);
}

TEST(TimingStreamMarkPause, MultiPair)
{
    std::vector<uint16_t> raw = { 833, 946, 1770, 897 };
    auto ts = TimingStream::fromMarkPause(raw);

    ASSERT_EQ(ts.timings().size(), 2u);
    EXPECT_EQ(ts.timings()[0].mark_us,  833u);
    EXPECT_EQ(ts.timings()[0].pause_us, 946u);
    EXPECT_EQ(ts.timings()[1].mark_us,  1770u);
    EXPECT_EQ(ts.timings()[1].pause_us, 897u);
}

// ---------------------------------------------------------------------------
// TimingStream::convertMarkPause
// Output: [mark0, pause0, mark1, pause1, ...]
// ---------------------------------------------------------------------------

TEST(TimingStreamConvertMarkPause, EmptyStream)
{
    auto ts = TimingStream::fromMarkPause({});
    EXPECT_TRUE(ts.convertMarkPause().empty());
}

TEST(TimingStreamConvertMarkPause, RoundTripFromMarkPause)
{
    std::vector<uint16_t> raw = { 833, 946, 1770, 897 };
    auto ts = TimingStream::fromMarkPause(raw);
    auto out = ts.convertMarkPause();

    ASSERT_EQ(out.size(), raw.size());
    EXPECT_EQ(out[0], 833u);
    EXPECT_EQ(out[1], 946u);
    EXPECT_EQ(out[2], 1770u);
    EXPECT_EQ(out[3], 897u);
}

TEST(TimingStreamConvertMarkPause, RoundTripFromMarkSegment)
{
    // fromMarkSegment stores mark+pause internally, convertMarkPause should
    // give back the same mark/pause values
    std::vector<uint16_t> raw = { 833, 1779, 1770, 2667 };
    auto ts = TimingStream::fromMarkSegment(raw);
    auto out = ts.convertMarkPause();

    ASSERT_EQ(out.size(), 4u);
    EXPECT_EQ(out[0], 833u);
    EXPECT_EQ(out[1], 946u);   // 1779-833
    EXPECT_EQ(out[2], 1770u);
    EXPECT_EQ(out[3], 897u);   // 2667-1770
}

TEST(TimingStreamConvertMarkPause, SilenceBlock)
{
    std::vector<uint16_t> raw = { 0, 32768 };
    auto ts = TimingStream::fromMarkSegment(raw);
    auto out = ts.convertMarkPause();

    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], 0u);
    EXPECT_EQ(out[1], 32768u);
}

// ---------------------------------------------------------------------------
// addMarkSegment / addMarkPause accumulate into the same stream
// ---------------------------------------------------------------------------

TEST(TimingStreamAccumulate, AddMarkSegmentTwice)
{
    TimingStream ts;
    ts.addMarkSegment({ 833, 1779 });
    ts.addMarkSegment({ 1770, 2667 });
    EXPECT_EQ(ts.timings().size(), 2u);
}

TEST(TimingStreamAccumulate, AddMarkPauseTwice)
{
    TimingStream ts;
    ts.addMarkPause({ 833, 946 });
    ts.addMarkPause({ 1770, 897 });
    EXPECT_EQ(ts.timings().size(), 2u);
}
