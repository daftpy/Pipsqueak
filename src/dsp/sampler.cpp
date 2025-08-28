//
// Created by Daftpy on 8/23/2025.
//
#include <algorithm>
#include <pipsqueak/dsp/sampler.hpp>

namespace pipsqueak::dsp {
    Sampler::Sampler(std::shared_ptr<const core::AudioBuffer> sampleData) : sampleData_(std::move(sampleData)) {
        nativeRate_ = 44100.0;
        engineRate_ = 48000.0;
        rootNote_ = 48;
        tuneCents_ = 0.0;
        maxPolyphony_ = 1;

        voices_.resize(maxPolyphony_);
        for (auto& v : voices_) {
            v.configure(sampleData_, nativeRate_, engineRate_);
        }
    }

    void Sampler::setEngineRate(const double rate) {
        engineRate_ = rate;
        for (auto& v : voices_) {
            v.configure(sampleData_, nativeRate_, engineRate_);
        }
    }

    void Sampler::setNativeRate(const double rate) {
        nativeRate_ = rate;
        for (auto& v : voices_) {
            v.configure(sampleData_, nativeRate_, engineRate_);
        }
    }

    void Sampler::setRootNote(const int note) {
        rootNote_ = note;
    }

    void Sampler::setTuneCents(const double cents) {
        tuneCents_ = cents;
    }

    void Sampler::process(core::AudioBuffer& buffer) {
        // Render each active voice into the buffer
        const auto n = static_cast<size_t>(buffer.numFrames());
        for (auto& v : voices_) {
            if (!v.finished()) {
                v.render(buffer, n);
            }
        }
    }

    bool Sampler::isFinished() const {
        return std::all_of(voices_.begin(), voices_.end(),
            [](const SamplerVoice& v) {
                return v.finished();
            });
    }

    void Sampler::noteOn(int note, float velocity) {
        // Find a free voice first
        for (auto& v : voices_) {
            if (v.finished()) {
                v.start(note, velocity, rootNote_, tuneCents_);
                return;
            }
        }

        // Simple voice-steal policy for step 1: reuse voice 0
        // (When you add polyphony >1, consider oldest/quietest steal.)
        if (!voices_.empty()) {
            voices_[0].start(note, velocity, rootNote_, tuneCents_);
        }
    }

    void Sampler::noteOff(int note) {
        // TODO: note off
    }
}
