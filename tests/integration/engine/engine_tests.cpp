//
// Created by Daftpy on 7/26/2025.
//
#include <gtest/gtest.h>
#include <pipsqueak/audio_io/device_scanner.hpp>
#include <pipsqueak/engine/engine.hpp>

/// Tests the engine can start a stream using the device id provided by the device manager.
TEST(EngineIntegrationTest, StartsStreamWithGivenDevice) {
    // ARRANGE: Create the device manager and engine objects
    pipsqueak::engine::AudioEngine engine;
    const pipsqueak::audio_io::DeviceScanner deviceManager(engine.audio());

    // ACT: Start the stream
    engine.startStream(deviceManager.currentDevice().value().ID, 44100, 512);

    // ASSERT: Check that engine's isRunning method returns true
    EXPECT_TRUE(engine.isRunning());
}

/// Tests the engine can stop a stream successfully.
TEST(EngineIntegrationTest, StopsStreamCorrectly) {
    // ARRANGE: Create the device manager and engine objects
    pipsqueak::engine::AudioEngine engine;
    const pipsqueak::audio_io::DeviceScanner deviceManager(engine.audio());

    // Start the stream and check it is running
    engine.startStream(deviceManager.currentDevice().value().ID, 44100, 512);
    ASSERT_TRUE(engine.isRunning());

    // ACT: Stop the stream
    engine.stopStream();

    // ASSERT: Check that engine's isRunning method returns false
    EXPECT_FALSE(engine.isRunning());
}