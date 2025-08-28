//
// Created by Daftpy on 8/23/2025.
//

#ifndef SAMPLER_VOICE_HPP
#define SAMPLER_VOICE_HPP

#include <pipsqueak/core/audio_buffer.hpp>

namespace pipsqueak::dsp {
    class SamplerVoice {
    public:
        SamplerVoice() = default;

        // Establish sample context
        void configure(std::shared_ptr<const core::AudioBuffer> sample, double nativeRate, double engineRate);

        // Start a note: compute step, reset phase, set gain/active
        void start(int note, float velocity, int rootNote, double tuneCents);

        // Render up to framesToRender
        void render(core::AudioBuffer& out, size_t framesToRender);

        [[nodiscard]] bool finished() const;

    private:
        // Sample context
        std::shared_ptr<const core::AudioBuffer> sample_{nullptr};
        unsigned srcChannels_{0};
        size_t numFrames_{0};
        size_t lastIndex_{0};
        double nativeRate_{0.0};
        double engineRate_{0.0};

        // Voice state
        double phase_{0.0};
        double step_{1.0};
        bool active_{false};
        float gain_{0.0};
    };
}
#endif //SAMPLER_VOICE_HPP
