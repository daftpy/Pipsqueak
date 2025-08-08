//
// Created by Daftpy on 8/8/2025.
//
#include <gtest/gtest.h>
#include <pipsqueak/dsp/mixer.hpp>
#include <pipsqueak/dsp/sample_player.hpp>
#include <pipsqueak/core/audio_buffer.hpp>

// Test that the mixer correctly sums the output of multiple sources.
TEST(MixerTest, SumsSourcesCorrectly) {
    // ARRANGE: Create the mixer and declare the number of frames
    pipsqueak::dsp::Mixer mixer;
    constexpr unsigned int numFrames = 16;

    // Create a source buffer filled with a constant value of 0.2
    auto sourceBuffer1 = std::make_shared<pipsqueak::core::AudioBuffer>(1, numFrames);
    sourceBuffer1->fill(0.2);
    auto player1 = std::make_shared<pipsqueak::dsp::SamplePlayer>(sourceBuffer1);
    player1->play();

    // Create a second source buffer filled with a constant value of 0.3
    auto sourceBuffer2 = std::make_shared<pipsqueak::core::AudioBuffer>(1, numFrames);
    sourceBuffer2->fill(0.3);
    auto player2 = std::make_shared<pipsqueak::dsp::SamplePlayer>(sourceBuffer2);
    player2->play();

    // Add both players to the mixer
    mixer.addSource(player1);
    mixer.addSource(player2);

    // Create an empty output buffer to process into
    pipsqueak::core::AudioBuffer outputBuffer(1, numFrames);
    outputBuffer.fill(0.0);

    // ACT: Process one block of audio
    mixer.process(outputBuffer);

    // ASSERT: The output buffer should contain the sum of the two sources (0.2 + 0.3 = 0.5)
    for (unsigned int i = 0; i < numFrames; ++i) {
        EXPECT_NEAR(outputBuffer.at(0, i), 0.5, 1e-9);
    }
}

// Test that after clearing the sources, the mixer produces silence.
TEST(MixerTest, ClearSourcesResultsInSilence) {
    // ARRANGE: Create the mixer and declare the number of frames
    pipsqueak::dsp::Mixer mixer;
    constexpr unsigned int numFrames = 16;

    // Add a source to the mixer
    auto sourceBuffer = std::make_shared<pipsqueak::core::AudioBuffer>(1, numFrames);
    sourceBuffer->fill(0.5);
    auto player = std::make_shared<pipsqueak::dsp::SamplePlayer>(sourceBuffer);
    player->play();
    mixer.addSource(player);

    // Create an empty output buffer
    pipsqueak::core::AudioBuffer outputBuffer(1, numFrames);
    outputBuffer.fill(0.0);

    // ACT: Clear all sources and then process one block of audio
    mixer.clearSources();
    mixer.process(outputBuffer);

    // ASSERT: The output buffer should still be silent because the mixer is empty.
    for (unsigned int i = 0; i < numFrames; ++i) {
        EXPECT_NEAR(outputBuffer.at(0, i), 0.0, 1e-9);
    }
}

// A stress test to verify that the atomic swap mechanism prevents data races
// between a "writer" thread (like a dispatcher) and a "reader" thread (like the audio callback).
TEST(MixerTest, ConcurrentReadWriteIsSafe) {
    // ARRANGE: Create the mixer, buffer and flag.
    pipsqueak::dsp::Mixer mixer;
    pipsqueak::core::AudioBuffer outputBuffer(1, 16); // A dummy buffer for the reader
    std::atomic<bool> stopFlag = false;

    // 1. The "writer" thread simulates a dispatcher rapidly adding and clearing sources.
    std::thread writerThread([&]() {
        while (!stopFlag) {
            // Create a brand new buffer and player on every iteration
            auto sourceBuffer = std::make_shared<pipsqueak::core::AudioBuffer>(1, 16);
            auto player = std::make_shared<pipsqueak::dsp::SamplePlayer>(sourceBuffer);
            player->play();

            mixer.addSource(player);
            mixer.clearSources();
        }
    });

    // 2. The "reader" thread simulates the real-time audio thread calling process().
    std::thread readerThread([&]() {
        while (!stopFlag) {
            mixer.process(outputBuffer);
        }
    });

    // ACT: Let the two threads run concurrently for a short duration.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // Signal both threads to stop.
    stopFlag = true;

    // Wait for both threads to finish their loops and exit.
    writerThread.join();
    readerThread.join();

    // ASSERT: If the test completes without crashing or throwing an exception, it
    // means the atomic swap successfully prevented any data races or memory
    // corruption. The test passes by not failing.
    SUCCEED();
}