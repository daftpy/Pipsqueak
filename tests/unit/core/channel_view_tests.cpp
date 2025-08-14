//
// Created by Daftpy on 8/14/2025.
//
#include <gtest/gtest.h>
#include <algorithm>
#include <type_traits>

#include <pipsqueak/core/audio_buffer.hpp>
#include <pipsqueak/core/channel_view.hpp>

using pipsqueak::core::AudioBuffer;
using pipsqueak::core::Sample;

/// ChannelView provides correct access to the channel data
TEST(ChannelViewTest, ProvidesCorrectAccess) {
    AudioBuffer buffer(2, 3);
    buffer.data() = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f};

    constexpr Sample newValueFrame1{0.99f};
    auto ch0 = buffer.channel(0);
    ch0[1] = newValueFrame1;

    EXPECT_FLOAT_EQ(buffer.at(0, 1), newValueFrame1); // modified
    EXPECT_FLOAT_EQ(buffer.at(1, 1), 0.3f);           // other channel unchanged
}

/// applyGain on a ChannelView modifies only that channel
TEST(ChannelViewTest, ApplyGainModifiesCorrectChannel) {
    constexpr unsigned int numFrames{3};
    AudioBuffer buffer(2, numFrames);

    for (size_t i = 0; i < numFrames; ++i) {
        buffer.at(0, i) = 0.5f;
        buffer.at(1, i) = 0.8f;
    }

    constexpr Sample gain{2.0f};
    buffer.channel(0).applyGain(gain);

    for (size_t i = 0; i < numFrames; ++i) {
        EXPECT_FLOAT_EQ(buffer.at(0, i), 1.0f);
        EXPECT_FLOAT_EQ(buffer.at(1, i), 0.8f);
    }
}

/// fill on a ChannelView modifies only that channel
TEST(ChannelViewTest, FillModifiesCorrectChannel) {
    AudioBuffer buffer(2, 10);

    constexpr Sample fillValue{0.77f};
    buffer.channel(1).fill(fillValue);

    for (size_t i = 0; i < buffer.numFrames(); ++i) {
        EXPECT_FLOAT_EQ(buffer.at(1, i), fillValue);
        EXPECT_FLOAT_EQ(buffer.at(0, i), 0.0f);
    }
}

/// copyFrom writes only to the selected channel
TEST(ChannelViewTest, CopyFromCopiesCorrectly) {
    AudioBuffer buffer(2, 4);
    const std::vector<Sample> src = {0.1f, 0.2f, 0.3f, 0.4f};

    buffer.channel(1).copyFrom(src.begin(), src.end());

    for (size_t i = 0; i < src.size(); ++i) {
        EXPECT_FLOAT_EQ(buffer.at(1, i), src[i]);
        EXPECT_FLOAT_EQ(buffer.at(0, i), 0.0f);
    }
}

/// raw() fast path lets us modify data via pointer+stride
TEST(ChannelViewTest, RawSpanWritableModifiesData) {
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{8};
    AudioBuffer buffer(numChannels, numFrames);
    buffer.fill(0.0f);

    auto ch1 = buffer.channel(1).raw(); // RawSpan<false>
    for (size_t i = 0; i < ch1.frames; ++i) {
        ch1.at(i) = 0.25f;              // uses stride internally
    }

    for (size_t i = 0; i < numFrames; ++i) {
        EXPECT_FLOAT_EQ(buffer.at(1, i), 0.25f);
        EXPECT_FLOAT_EQ(buffer.at(0, i), 0.0f);
    }
}

/// const raw() returns a const pointer (compile-time check) and reads correctly
TEST(ChannelViewTest, RawSpanConstIsConstAndReadable) {
    AudioBuffer buffer(2, 4);
    for (size_t i = 0; i < buffer.numFrames(); ++i) {
        buffer.at(0, i) = 0.1f * static_cast<float>(i + 1);
    }

    const AudioBuffer& cbuf = buffer;       // force ReadOnlyChannelView
    auto span = cbuf.channel(0).raw();      // RawSpan<true>

    static_assert(std::is_const_v<std::remove_pointer_t<decltype(span.ptr)>>,
                  "raw() on const view must expose const Sample*");

    for (size_t i = 0; i < span.frames; ++i) {
        EXPECT_FLOAT_EQ(span.at(i), 0.1f * static_cast<float>(i + 1));
    }
}

/// Iterator (writable): range-for writes only the targeted channel
TEST(ChannelViewTest, IteratorWritableRangeForModifiesOnlyThatChannel) {
    constexpr unsigned int numFrames{6};
    AudioBuffer buffer(2, numFrames);
    buffer.fill(0.0f);

    // Write via strided iterator
    for (auto& s : buffer.channel(1)) {
        s = 0.25f;
    }

    for (size_t i = 0; i < numFrames; ++i) {
        EXPECT_FLOAT_EQ(buffer.at(1, i), 0.25f); // modified channel
        EXPECT_FLOAT_EQ(buffer.at(0, i), 0.0f);  // untouched channel
    }
}

/// Iterator (const): range-for reads all frames with expected values
TEST(ChannelViewTest, IteratorConstRangeForReadsAllFrames) {
    constexpr unsigned int numFrames{5};
    AudioBuffer buffer(2, numFrames);

    // Fill channel 0 with an arithmetic sequence
    for (size_t i = 0; i < numFrames; ++i) {
        buffer.at(0, i) = 0.1f * static_cast<float>(i + 1); // 0.1, 0.2, ...
    }

    const AudioBuffer& cbuf = buffer;
    float sum = 0.0f;
    for (auto s : cbuf.channel(0)) { // const iterator yields by value (const ref)
        sum += s;
    }

    const float expected = 0.1f * (1 + numFrames) * (numFrames / 2.0f); // sum of 0.1..0.1*n
    EXPECT_FLOAT_EQ(sum, expected);
}

/// Iterator: pointer stride equals interleave stride (verifies ++ steps correctly)
TEST(ChannelViewTest, IteratorStrideMatchesInterleave) {
    AudioBuffer buffer(3, 4); // 3 channels, 4 frames
    auto view = buffer.channel(2);

    auto it0 = view.begin();
    auto it1 = it0; ++it1;

    // Taking addresses of dereferenced refs to compare pointer distance in Samples
    const auto* p0 = &(*it0);
    const auto* p1 = &(*it1);

    const ptrdiff_t elemStride = p1 - p0; // measured in Sample elements
    EXPECT_EQ(elemStride, static_cast<ptrdiff_t>(buffer.interleaveStride()));
}
