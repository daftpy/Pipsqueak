//
// Created by Daftpy on 7/25/2025.
//

#include "audio_io/device_manager.hpp"

namespace pipsqueak::audio_io {
    DeviceManager::DeviceManager(RtAudio& audio) : audio_(audio) {
        std::cout << "DeviceManager initialized!\n";

        if (findDefaultDevice())
            std::cout << "DeviceManager: a useable device was found\n";
    }

    std::optional<RtAudio::DeviceInfo> DeviceManager::currentDevice() const {
        return currentDevice_;
    }

    bool DeviceManager::findDefaultDevice() {
        // Check if there are any devices at all.
        if (audio_.getDeviceCount() < 1) {
            std::cout << "No audio devices found.\n";
            currentDevice_ = std::nullopt;
            return false;
        }

        // Get the default output device ID. We can now trust it's valid.
        const unsigned int deviceId = audio_.getDefaultOutputDevice();
        auto info = audio_.getDeviceInfo(deviceId);

        currentDevice_ = info;

        // Log details about the selected device
        std::cout << "Selected output device: " << info.name << "\n"
                  << "  Output Channels: " << info.outputChannels << "\n"
                  << "  Sample Rates: ";

        for (const auto &rate : info.sampleRates) {
            std::cout << rate << " ";
        }
        std::cout << "\n";

        return true;
    }
}