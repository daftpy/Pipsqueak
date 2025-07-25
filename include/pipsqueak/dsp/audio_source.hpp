//
// Created by Daftpy on 7/24/2025.
//

#ifndef AUDIO_SOURCE_HPP
#define AUDIO_SOURCE_HPP

#include "pipsqueak/core/audio_buffer.hpp"

namespace pipsqueak::dsp {
    class AudioSource {
    public:
        virtual ~AudioSource() = default;

        /**
         * @brief Process audio, filling the provided buffer with sound.
         * @param buffer The output buffer to fill with audio data.
         */
        virtual void process(core::AudioBuffer& buffer) = 0;

        virtual bool isFinished() const = 0;
    };
}

#endif //AUDIO_SOURCE_HPP
