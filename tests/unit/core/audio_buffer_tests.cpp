// Created by Daftpy on 7/23/2025.
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>

#include <pipsqueak/core/audio_buffer.hpp>
#include <pipsqueak/core/channel_view.hpp>

using pipsqueak::core::AudioBuffer;
using pipsqueak::core::Sample;

static_assert(sizeof(Sample) == 4, "Sample must be 32-bit float");

/// Tests that an audio buffer is initialized with the correct state
TEST(AudioBufferTest, ConstructorInitializesStateCorrectly) {
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{512};

    const AudioBuffer buffer(numChannels, numFrames);

    EXPECT_EQ(buffer.numChannels(), numChannels);
    EXPECT_EQ(buffer.numFrames(), numFrames);
    EXPECT_EQ(buffer.data().size(), static_cast<size_t>(numChannels) * numFrames);
}

/// Tests that a specific sample can be accessed inside the audio buffer
TEST(AudioBufferTest, AtMethodProvidesCorrectAccess) {
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{10};
    AudioBuffer buffer(numChannels, numFrames);

    constexpr unsigned int testChannel{1};
    constexpr unsigned int testFrame{4};
    constexpr unsigned int testIndex{(testFrame * numChannels) + testChannel};
    constexpr Sample expectedValue{0.99f};

    buffer.data()[testIndex] = expectedValue;
    EXPECT_FLOAT_EQ(buffer.at(testChannel, testFrame), expectedValue);
}

/// Tests that the at() method throws an exception for invalid channel index
TEST(AudioBufferTest, AtMethodThrowsOnInvalidChannel) {
    AudioBuffer buffer(2, 10);
    ASSERT_THROW(buffer.at(2, 5), std::out_of_range);
    ASSERT_THROW(buffer.at(2, 5), std::out_of_range);
}

/// Tests that applyGain() affects all channels (single-pass implementation)
TEST(AudioBufferTest, BufferApplyGainModifiesAllChannels) {
    AudioBuffer buffer(2, 10);
    std::fill(buffer.data().begin(), buffer.data().end(), 0.5f);

    constexpr Sample gainFactor{2.0f};
    constexpr Sample expectedValue{1.0f};

    buffer.applyGain(gainFactor);

    for (const auto v : buffer.data()) {
        EXPECT_FLOAT_EQ(v, expectedValue);
    }
}

/// Tests that fill() affects all channels (single-pass implementation)
TEST(AudioBufferTest, BufferFillModifiesAllChannels) {
    AudioBuffer buffer(2, 10);
    std::fill(buffer.data().begin(), buffer.data().end(), 0.0f);

    constexpr Sample fillValue{0.99f};
    buffer.fill(fillValue);

    for (const auto v : buffer.data()) {
        EXPECT_FLOAT_EQ(v, fillValue);
    }
}

/// Tests that copyFrom() correctly copies interleaved data into the buffer
TEST(AudioBufferTest, BufferCopyFromCopiesCorrectly) {
    AudioBuffer buffer(2, 3);
    const std::vector<Sample> sourceData{0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f};

    buffer.copyFrom(sourceData.begin(), sourceData.end());
    EXPECT_EQ(buffer.data(), sourceData);
}

/// Test that copyFrom() truncates copied data if it overflows the buffer
TEST(AudioBufferTest, BufferCopyFromTruncatesOverflowData) {
    AudioBuffer buffer(2, 3);
    const std::vector<Sample> sourceData{0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f};
    const size_t originalSize{buffer.data().size()};

    buffer.copyFrom(sourceData.begin(), sourceData.end());
    EXPECT_EQ(originalSize, buffer.data().size());
}

/// Unchecked access returns same elements as checked access (for valid indices)
TEST(AudioBufferTest, UncheckedAccessMatchesChecked) {
    constexpr unsigned int ch{1};
    constexpr unsigned int fr{2};
    AudioBuffer buffer(2, 8);

    // Set via checked
    buffer.at(ch, fr) = 0.33f;

    // Read via unchecked
    EXPECT_FLOAT_EQ(buffer.at_unchecked(ch, fr), 0.33f);

    // Write via unchecked and read via checked
    buffer.at_unchecked(ch, fr) = 0.77f;
    EXPECT_FLOAT_EQ(buffer.at(ch, fr), 0.77f);
}

/// dataPtr + stride math lines up with interleaving
TEST(AudioBufferTest, DataPtrAndStrideAreCorrect) {
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{4};
    AudioBuffer buffer(numChannels, numFrames);

    Sample* base = buffer.dataPtr();
    const unsigned int stride = buffer.interleaveStride();

    // Write channel 1, frame 3 via pointer/stride
    base[3 * stride + 1] = 0.5f;

    EXPECT_FLOAT_EQ(buffer.at(1, 3), 0.5f);
    EXPECT_EQ(stride, numChannels);
}
