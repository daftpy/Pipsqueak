//
// Created by Daftpy on 7/29/2025.
//

#ifndef AUDIO_IO_TYPES_HPP
#define AUDIO_IO_TYPES_HPP

#include <string>
#include <utility>
#include <vector>

namespace pipsqueak::audio_io {
    struct AudioDevice {
        unsigned int ID;
        std::string name;
        std::vector<unsigned int> availableSampleRates;
        unsigned int outputChannels;
        bool isDefaultOutput;

        AudioDevice()
            : ID(0),
              name("Invalid Device"),
              availableSampleRates({}),
              outputChannels(0),
              isDefaultOutput(false) {}

        AudioDevice(const unsigned int id, std::string  deviceName, const std::vector<unsigned int>& availableRates, const unsigned int maxChannels, const bool isDefault)
             : ID(id), name(std::move(deviceName)), availableSampleRates(availableRates), outputChannels(maxChannels), isDefaultOutput(isDefault) {}
    };
}

#endif //AUDIO_IO_TYPES_HPP
