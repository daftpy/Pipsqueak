//
// Created by Daftpy on 7/25/2025.
//

#include "audio_io/device_manager.hpp"

#include "core/logging.hpp"

namespace pipsqueak::audio_io {
    DeviceManager::DeviceManager(RtAudio& audio) : audio_(audio) {
        core::logging::Logger::log("pipsqueak", "DeviceManager initialized!");

        if (findDefaultDevice())
            core::logging::Logger::log("pipsqueak", "DeviceManager: a useable device was found!");
    }

    std::optional<RtAudio::DeviceInfo> DeviceManager::currentDevice() const {
        return currentDevice_;
    }

    bool DeviceManager::findDefaultDevice() {
        // Check if there are any devices at all.
        if (audio_.getDeviceCount() < 1) {
            core::logging::Logger::log("pipsqueak", "DeviceManager: no usable device was found!");

            currentDevice_ = std::nullopt;
            return false;
        }

        // Get the default output device ID. We can now trust it's valid.
        const unsigned int deviceId = audio_.getDefaultOutputDevice();
        auto info = audio_.getDeviceInfo(deviceId);

        currentDevice_ = info;

        // Log details about the selected device
        std::string message = "Selected output device: " + info.name + "\n"
            + " Output Channels: " + std::to_string(info.outputChannels) + "\n"
            + " Sample Rates: ";

        for (const auto &rate : info.sampleRates) {
            std::string rateString = std::to_string(rate) + " ";
            message.append(rateString);
        }

        core::logging::Logger::log("pipsqueak", message);

        return true;
    }
}
