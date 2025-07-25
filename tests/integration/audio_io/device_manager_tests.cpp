//
// Created by Daftpy on 7/25/2025.
//
#include <gtest/gtest.h>
#include <pipsqueak/audio_io/device_manager.hpp>
#include <RtAudio.h>

/// An integration test that checks basic interaction with a real RtAudio instance.
TEST(DeviceManagerIntegrationTest, FindDefaultDeviceBehavesLogically) {
    // ARRANGE: Create a real RtAudio instance and a DeviceManager.
    RtAudio audio;
    pipsqueak::audio_io::DeviceManager manager(audio);

    // On initialization, the manager should attempt to find the default audio device

    // ASSERT: Check for logical consistency based on the system state.
    if (audio.getDeviceCount() < 1) {
        // If there are no devices, we expect the search to fail.
        EXPECT_FALSE(manager.currentDevice().has_value());
    } else {
        // If there ARE devices, we expect the search to succeed.
        ASSERT_TRUE(manager.currentDevice().has_value());
        // And the found device should have output channels.
        EXPECT_GT(manager.currentDevice()->outputChannels, 0);
    }
}