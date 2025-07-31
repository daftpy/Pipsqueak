//
// Created by Daftpy on 7/25/2025.
//

#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include <RtAudio.h>
#include <optional>
#include <vector>

#include "types.hpp"

namespace pipsqueak::audio_io {
    /**
     * @class DeviceScanner
     * @brief A utility class for querying and selecting audio hardware devices.
     * It inspects an RtAudio instance but does not own it.
     */
    class DeviceScanner {
    public:
        /**
         * @brief Constructs a DeviceManager and attempts to find a default output device.
         * @param audio A reference to an active RtAudio instance.
         */
        explicit DeviceScanner(RtAudio& audio);

        /**
         * @brief Gets information about the currently selected audio device.
         * @return An optional containing the device info, or std::nullopt if no device is selected.
         */
        [[nodiscard]] std::optional<AudioDevice> defaultDevice() const;

        /**
         * @brief Gets a collection of available audio devices.
         * @return A vector of device info, each representing an available device.
         */
        [[nodiscard]] std::vector<AudioDevice> allUsableDevices() const;

        /**
         * @brief Gets information about an audio device by its device id.
         * @return Device info retrieved with the requested device id.
         */
        [[nodiscard]] AudioDevice deviceInfo(unsigned int id) const;

    private:
        /**
         * @brief Scans for and selects the system's default output device.
         * @return True if a usable device was found and selected, false otherwise.
         */
        bool findDefaultDevice();

        // A reference to the RtAudio instance owned by the engine
        RtAudio& audio_;

        // Holds information about the selected device.
        std::optional<RtAudio::DeviceInfo> currentDevice_{std::nullopt};
    };
}

#endif //DEVICE_MANAGER_HPP
