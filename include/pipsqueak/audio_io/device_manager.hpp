//
// Created by Daftpy on 7/25/2025.
//

#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include <RtAudio.h>
#include <optional>

namespace pipsqueak::audio_io {
    /**
     * @class DeviceManager
     * @brief A utility class for querying and selecting audio hardware devices.
     * It inspects an RtAudio instance but does not own it.
     */
    class DeviceManager {
    public:
        /**
         * @brief Constructs a DeviceManager and attempts to find a default output device.
         * @param audio A reference to an active RtAudio instance.
         */
        explicit DeviceManager(RtAudio& audio);

        /**
         * @brief Gets information about the currently selected audio device.
         * @return An optional containing the device info, or std::nullopt if no device is selected.
         */
        std::optional<RtAudio::DeviceInfo> currentDevice() const;

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
