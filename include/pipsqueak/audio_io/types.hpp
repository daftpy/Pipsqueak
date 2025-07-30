//
// Created by Daftpy on 7/29/2025.
//

#ifndef AUDIO_IO_TYPES_HPP
#define AUDIO_IO_TYPES_HPP

#include <string>
#include <utility>

namespace pipsqueak::audio_io {
    struct AudioDevice {
        unsigned int ID;
        std::string name;
        bool isDefaultOutput;
        unsigned int outputChannels;

        AudioDevice(const unsigned int id, std::string  deviceName, const bool isDefault, const unsigned int maxChannels)
             : ID(id), name(std::move(deviceName)), isDefaultOutput(isDefault), outputChannels(maxChannels) {}
    };
}

#endif //AUDIO_IO_TYPES_HPP
