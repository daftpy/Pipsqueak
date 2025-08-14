//
// Created by Daftpy on 7/25/2025.
//

#include <gtest/gtest.h>
#include <pipsqueak/dsp/sample_player.hpp>
#include <pipsqueak/core/audio_buffer.hpp>
#include <pipsqueak/core/channel_view.hpp>
#include <memory>

// A helper to create a dummy buffer for testing
std::shared_ptr<pipsqueak::core::AudioBuffer> createDummyBuffer(unsigned int channels, unsigned int frames) {
    return std::make_shared<pipsqueak::core::AudioBuffer>(channels, frames);
}

/// Tests that a newly created SamplePlayer is in a non-playing state
TEST(SamplePlayerTest, InitialStateIsInactive) {
    // ARRANGE: Create a player with some dummy sample data
    auto dummySample = createDummyBuffer(1, 100);
    pipsqueak::dsp::SamplePlayer player(dummySample);

    // ACT & ASSERT: The player should be "finished".
    EXPECT_TRUE(player.isFinished());
}

/// Tests that a non-playing SamplePlayer produces silence
TEST(SamplePlayerTest, InactivePlayerDoesNotModifyBuffer) {
    // ARRANGE: Create a player with an empty buffer and output buffer filled with non-zero data.
    auto dummySample = createDummyBuffer(1, 100);
    pipsqueak::dsp::SamplePlayer player(dummySample);
    pipsqueak::core::AudioBuffer outputBuffer(2, 256);
    outputBuffer.fill(0.5); // Fill with a non-zero value.

    const std::vector<float> originalData = outputBuffer.data();

    // ACT: Process audio with the inactive player.
    player.process(outputBuffer);

    // ASSERT: The output buffer should clear to silence
    EXPECT_EQ(outputBuffer.data(), originalData);
}

/// Tests that calling play() correctly activates the player.
TEST(SamplePlayerTest, PlayStartsPlaybackAndResetsPosition) {
    // ARRANGE: Create a player and verify it is not playing.
    auto dummySample = createDummyBuffer(1, 100);
    pipsqueak::dsp::SamplePlayer player(dummySample);

    // Verify the player is finished (not playing)
    ASSERT_TRUE(player.isFinished());
    ASSERT_EQ(player.getPosition(), 0);

    // Manually set the position to be non-default
    player.setPosition(50); // Move the playhead
    ASSERT_EQ(player.getPosition(), 50);

    // ACT: Start playback.
    player.play();

    // ASSERT: The player should no longer be finished (its playing)
    EXPECT_FALSE(player.isFinished());
    EXPECT_EQ(player.getPosition(), 0); // Starts from the beginning
}

/// Tests that calling stop() correctly stops the player.
TEST(SamplePlayerTest, PlayerStopsPlaybackAndResetsPosition) {
    // ARRANGE: Create a player and start playing.
    auto dummySample = createDummyBuffer(1, 100);
    pipsqueak::dsp::SamplePlayer player(dummySample);
    player.play();

    // Verify the player is not finished (playing)
    ASSERT_FALSE(player.isFinished());

    // ACT: Stop playback
    player.stop();

    // ASSERT: The playback has stopped.
    EXPECT_TRUE(player.isFinished());
    EXPECT_EQ(player.getPosition(), 0); // Position reset.
}

/// Tests that process() correctly applies the audio buffer data to the output buffer.
TEST(SamplePlayerTest, ProcessCopiesMonoSourceToStereoOutput) {
    // ARRANGE: Create a mono source sample and a stereo output buffer
    auto sampleData = createDummyBuffer(1, 512);
    sampleData->fill(0.77); // Fill with a known value

    pipsqueak::dsp::SamplePlayer player(sampleData);
    pipsqueak::core::AudioBuffer outputBuffer(2, 256); // Stereo output

    // ACT: Start the player and process one block of audio.
    player.play();
    player.process(outputBuffer);

    // ASSERT: Check that the mono source was copied to both stereo channels.
    for (size_t f{0}; f < outputBuffer.numFrames(); ++f) {
        EXPECT_FLOAT_EQ(outputBuffer.at(0, f), 0.77); // Left channel
        EXPECT_FLOAT_EQ(outputBuffer.at(1, f), 0.77); // Right channel
    }
}

/// Tests that process() correctly copies a stereo source to a stereo output.
TEST(SamplePlayerTest, ProcessCopiesStereoSourceToStereoOutput) {
    // ARRANGE: Create a stereo source sample with distinct channel data.
    auto sampleData = createDummyBuffer(2, 512);
    sampleData->channel(0).fill(0.5);  // Fill Left channel with 0.5
    sampleData->channel(1).fill(-0.5); // Fill Right channel with -0.5

    pipsqueak::dsp::SamplePlayer player(sampleData);
    pipsqueak::core::AudioBuffer outputBuffer(2, 256); // Stereo output

    // ACT: Start the player and process one block of audio.
    player.play();
    player.process(outputBuffer);

    // ASSERT: Check that each channel was copied correctly.
    for (size_t f = 0; f < outputBuffer.numFrames(); ++f) {
        EXPECT_FLOAT_EQ(outputBuffer.at(0, f), 0.5);  // Left should be 0.5
        EXPECT_FLOAT_EQ(outputBuffer.at(1, f), -0.5); // Right should be -0.5
    }
}