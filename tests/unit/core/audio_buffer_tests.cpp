//
// Created by Daftpy on 7/23/2025.
//
#include <gtest/gtest.h>
#include <numeric>
#include <pipsqueak/core/audio_buffer.hpp>
#include <pipsqueak/core/channel_view.hpp>

/// Tests that an audio buffer is initialized with the correct state
TEST(AudioBufferTest, ConstructorInitializesStateCorrectly) {
    // ARRANGE: Define the dimensions
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{512};

    // ACT: Create the buffer
    const pipsqueak::core::AudioBuffer buffer(numChannels, numFrames);

    // ASSERT: Verify the state
    EXPECT_EQ(buffer.numChannels(), numChannels);
    EXPECT_EQ(buffer.numFrames(), numFrames);
    EXPECT_EQ(buffer.data().size(), numChannels * numFrames);
}

/// Tests that a specific sample can be accessed inside the audio buffer
TEST(AudioBufferTest, AtMethodProvidesCorrectAccess) {
    // ARRANGE: Create the buffer
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{10};
    pipsqueak::core::AudioBuffer buffer(numChannels, numFrames);

    // Calculate the index to set a sample for the test.
    // Index for (channel 1, frame 4) in an interleaved buffer
    // is (frame * number of channels) + channel.
    constexpr unsigned int testChannel{1};
    constexpr unsigned int testFrame{4};
    constexpr unsigned int testIndex{(testFrame * numChannels) + testChannel};
    constexpr double expectedValue{0.99};

    // Use the writable data() method to set up the test value
    buffer.data()[testIndex] = expectedValue;

    // ACT & ASSERT: The at(channel, frame) method should return the
    // value that was just set.
    EXPECT_DOUBLE_EQ(buffer.at(testChannel, testFrame), expectedValue);
}

/// Tests that the at() method throws an exceptin for invalid channel index
TEST(AudioBufferTest, AtMethodThrowsOnInvalidChannel) {
    // ARRANGE: Set up the buffer
    pipsqueak::core::AudioBuffer buffer(2, 10);

    // ACT & ASSERT: Verify that both versions throw on out_of_range
    ASSERT_THROW(buffer.at(2, 5), std::out_of_range);
    ASSERT_THROW(buffer.at(2, 5), std::out_of_range);
}

/// Tests that applyGain() on the AudioBuffer affects all channels.
TEST(AudioBufferTest, BufferApplyGainModifiesAllChannels) {
    // ARRANGE: Create a buffer and fill its channels with a known value.
    pipsqueak::core::AudioBuffer buffer(2, 10);
    std::fill(buffer.data().begin(), buffer.data().end(), 0.5);

    constexpr double gainFactor{2.0};
    constexpr double expectedValue{1.0};

    // ACT: Apply gain to the entire buffer.
    buffer.applyGain(gainFactor);

    // ASSERT: Check that all samples in all channels were modified.
    for (size_t i{0}; i < buffer.data().size(); ++i) {
        EXPECT_DOUBLE_EQ(buffer.data()[i], expectedValue);
    }
}

/// Tests that copyFrom() correctly copies interleaved data into the buffer.
TEST(AudioBufferTest, BufferCopyFromCopiesCorrectly) {
    // ARRANGE: Create a destination buffer and a source vector.
    pipsqueak::core::AudioBuffer buffer(2, 3);
    const std::vector<double> sourceData{0.1, 0.2, 0.3, 0.4, 0.5, 0.6};

    // ACT: Copy the source data into the buffer.
    buffer.copyFrom(sourceData.begin(), sourceData.end());

    // ASSERT: The buffer's data should match the source data.
    EXPECT_EQ(buffer.data(), sourceData);
}

/// Test that copyFrom() truncates copied data if it overflows the buffer.
TEST(AudioBufferTest, BufferCopyFromTruncatesOverflowData) {
    // ARRANGE: Create a destination buffer and a source vector.
    pipsqueak::core::AudioBuffer buffer(2, 3);
    const std::vector<double> sourceData{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
    const size_t originalSize{buffer.data().size()};

    // ACT: Copy the source data into the buffer.
    buffer.copyFrom(sourceData.begin(), sourceData.end());

    // ASSERT: The buffer's data size remains the same.
    EXPECT_EQ(originalSize, buffer.data().size());
}

/// Tests that fill() on the AudioBuffer affects all channels.
TEST(AudioBufferTest, BufferFillModifiesAllChannels) {
    // ARRANGE: Create a buffer and fill its channels with 0.0
    pipsqueak::core::AudioBuffer buffer(2, 10);

    // Fill the buffer with empty data
    std::fill(buffer.data().begin(), buffer.data().end(), 0.0);

    // Define a fill value
    constexpr double fillValue{0.99};

    // ACT: Fill buffer with a new value
    buffer.fill(fillValue);

    // ASSERT: Check that all samples in the buffer were modified
    for (size_t i{0}; i < buffer.data().size(); ++i) {
        EXPECT_DOUBLE_EQ(buffer.data()[i], fillValue);
    }
}

/// Tests a ChannelView provides correct access to the channel data
TEST(AudioBufferTest, ChannelViewProvidesCorrectAccess) {
    // ARRANGE: Create a buffer and fill it with data
    pipsqueak::core::AudioBuffer buffer(2, 3);
    buffer.data() = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5};
    constexpr double newValueFame1{0.99};

    // ACT: Get a view of channel 0 and modify a sample through it
    pipsqueak::core::ChannelView channelView = buffer.channel(0);
    channelView[1] = newValueFame1;

    // ASSERT: Check that the original buffer was modified
    EXPECT_DOUBLE_EQ(buffer.at(0, 1), newValueFame1);

    // ASSERT: Other channels were not affected
    EXPECT_DOUBLE_EQ(buffer.at(1, 1), 0.3);
}

/// Tests that applyGain on a ChannelView only affects that channel.
TEST(AudioBufferTest, ChannelViewApplyGainModifiesCorrectChannel) {
    // ARRANGE: Create a buffer and fill its channels with different values
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{3};
    pipsqueak::core::AudioBuffer buffer(numChannels, numFrames);

    // Fill channel 0 with 0.5 and channel 1 with 0.8
    for (size_t i{0}; i < numFrames; ++i) {
        buffer.at(0, i) = 0.5;
        buffer.at(1, i) = 0.8;
    }

    constexpr double gainFactor{2.0};
    constexpr double expectedChannel0Value{1.0};
    constexpr double expectedChannel1Value{0.8};

    // ACT: Apply gain to channel 0 ONLY
    buffer.channel(0).applyGain(gainFactor);

    // ASSERT: Check that all samples in channel 0 were modified correctly
    for (size_t i{0}; i < numFrames; ++i) {
        EXPECT_DOUBLE_EQ(buffer.at(0, i), expectedChannel0Value);
    }

    // ASSERT: Check that all samples in channel 1 were not modified
    for (size_t i{0}; i < numFrames; ++i) {
        EXPECT_DOUBLE_EQ(buffer.at(1, i), expectedChannel1Value);
    }
}


TEST(AudioBufferTest, ChannelViewFillModifiesCorrectChannel) {
    // ARRANGE: Create a buffer
    constexpr unsigned int numChannels{2};
    constexpr unsigned int numFrames{10};
    pipsqueak::core::AudioBuffer buffer(numChannels, numFrames);

    // Define the fill value
    constexpr double fillValue{0.77};

    // ACT: Fill channel 1 only
    buffer.channel(1).fill(fillValue);

    // ASSERT: Check that channel 1 was filled correctly
    for (size_t i{0}; i < numFrames; ++i) {
        EXPECT_DOUBLE_EQ(buffer.at(1, i), fillValue);
    }

    // ASSERT: Check that channel 0 was not modified
    for (size_t i{0}; i < numFrames; ++i) {
        EXPECT_DOUBLE_EQ(buffer.at(0, i), 0.0);
    }
}

TEST(AudioBufferTest, ChannelViewCopyFromCopiesCorrectly) {
    // ARRANGE: Create a destination buffer and source buffer.
    pipsqueak::core::AudioBuffer buffer(2, 4);
    const std::vector<double> sourceData = {0.1, 0.2, 0.3, 0.4};

    // ACT: Copy the data into channel 1 of the buffer.
    buffer.channel(1).copyFrom(sourceData.begin(), sourceData.end());

    // ASSERT: Check that the data was copied to the correct channel.
    for (size_t i{0}; i < sourceData.size(); ++i) {
        EXPECT_DOUBLE_EQ(buffer.at(1, i), sourceData[i]);
    }

    // ASSERT: Check that channel 0 was not modified.
    for (size_t i{0}; i < sourceData.size(); ++i) {
        EXPECT_DOUBLE_EQ(buffer.at(0, i), 0.0);
    }
}
