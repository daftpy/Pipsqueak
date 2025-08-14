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
