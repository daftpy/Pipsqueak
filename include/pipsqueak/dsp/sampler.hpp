//
// Created by Daftpy on 8/22/2025.
//

#ifndef SAMPLER_HPP
#define SAMPLER_HPP
#include "audio_source.hpp"
#include "pipsqueak/core/audio_buffer.hpp"
#include <memory>

#include "sampler_voice.hpp"

namespace pipsqueak::dsp {
    class Sampler final : public AudioSource {
    public:
        explicit Sampler(std::shared_ptr<const core::AudioBuffer> sampleData);

        void setEngineRate(double rate);
        void setNativeRate(double rate);
        void setRootNote(int note);
        void setTuneCents(double cents);

        /**
         * @brief Renders the next block of audio into the output buffer.
         * @param buffer The buffer to mix audio into.
         */
        void process(core::AudioBuffer& buffer) override;

        /**
         * @brief Checks if the sampler is currently inactive.
         * @return True if not playing, false otherwise.
         */
        [[nodiscard]] bool isFinished() const override;

        // Instrument API
        void noteOn(int note, float velocity);
        void noteOff(int note);

    private:
        // The shared audio data this sampler will read from.
        std::shared_ptr<const core::AudioBuffer> sampleData_;

        double engineRate_{48000.0};
        double nativeRate_{44100.0};

        int rootNote_{48}; // C3
        double tuneCents_{0.0};

        size_t maxPolyphony_{1};
        std::vector<SamplerVoice> voices_;
    };
}

#endif //SAMPLER_HPP
