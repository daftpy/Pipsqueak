//
// Created by Daftpy on 8/23/2025.
//

#include <algorithm>
#include <pipsqueak/dsp/sampler_voice.hpp>
#include <pipsqueak/core/channel_view.hpp>

namespace pipsqueak::dsp {
    void SamplerVoice::configure(std::shared_ptr<const core::AudioBuffer> sample, double nativeRate, double engineRate) {
        sample_     = std::move(sample);
        nativeRate_ = nativeRate;
        engineRate_ = engineRate;

        if (sample_) {
            srcChannels_ = sample_->numChannels();
            numFrames_   = sample_->numFrames();
            lastIndex_   = numFrames_ ? (numFrames_ - 1) : 0;
        } else {
            srcChannels_ = 0;
            numFrames_   = 0;
            lastIndex_   = 0;
        }
    }

    void SamplerVoice::start(const int note, const float velocity, const int rootNote, const double tuneCents) {
        // Check sample and values are valid
        if (!sample_ || numFrames_ < 2 || nativeRate_ <= 0.0 || engineRate_ <= 0.0) {
            active_ = false;
            return;
        }

        const auto semis = static_cast<double>(note - rootNote);
        const double pitchScale = std::pow(2.0, semis / 12.0) * std::pow(2.0, tuneCents / 1200.0);

        step_ = (nativeRate_ / engineRate_) * pitchScale;
        phase_ = 0.0;

        // Simple velocity to gain mapping for now (linear 0..1)
        gain_ = std::clamp(velocity, 0.0f, 1.0f);
        active_ = (step_ > 0.0);
    }

    void SamplerVoice::render(core::AudioBuffer& out, size_t framesToRender) {
        // Bail out early if the voice isn't active, there's no sample, or there's nothing to render.
        if (!active_ || !sample_ || framesToRender == 0)
            return;

        // Query output channel count. If either output or source has 0 channels, stop this voice.
        const unsigned outCh = out.numChannels();
        if (outCh == 0 || srcChannels_ == 0) {
            active_ = false;
            return;
        }

        // ---- Gather per-channel spans (views) once for this call ----
        // Build a list of source-channel spans up to the number we can actually copy.
        std::vector<decltype(sample_->channel(0).raw())> srcSpans;
        const unsigned nCopy = std::min(outCh, srcChannels_);
        srcSpans.reserve(nCopy);
        for (unsigned c = 0; c < nCopy; ++c)
            srcSpans.push_back(sample_->channel(c).raw());

        // Build output-channel spans (we may have more outs than source channels).
        std::vector<decltype(out.channel(0).raw())> outSpans;
        outSpans.reserve(outCh);
        for (unsigned c = 0; c < outCh; ++c)
            outSpans.push_back(out.channel(c).raw());

        // If the source is mono, we'll duplicate the same interpolated value to all output channels.
        const bool monoSrc = (srcChannels_ == 1);
        const auto monoSpan = monoSrc ? sample_->channel(0).raw()
                                      : decltype(sample_->channel(0).raw()){}; // empty if not mono

        // ---- Per-frame render loop ----
        for (size_t f = 0; f < framesToRender; ++f) {
            const auto i = static_cast<size_t>(phase_);
            if (i > lastIndex_) { active_ = false; break; }

            const double frac = phase_ - static_cast<double>(i);

            if (monoSrc) {
                core::Sample s;
                if (i == lastIndex_) {
                    s = monoSpan.at(i);
                } else {
                    const core::Sample x0 = monoSpan.at(i);
                    const core::Sample x1 = monoSpan.at(i + 1);
                    s = static_cast<core::Sample>(x0 + (x1 - x0) * frac);
                }
                for (unsigned c = 0; c < outCh; ++c) outSpans[c].at(f) += gain_ * s;
            } else {
                for (unsigned c = 0; c < nCopy; ++c) {
                    core::Sample s;
                    if (i == lastIndex_) {
                        s = srcSpans[c].at(i);
                    } else {
                        const core::Sample x0 = srcSpans[c].at(i);
                        const core::Sample x1 = srcSpans[c].at(i + 1);
                        s = static_cast<core::Sample>(x0 + (x1 - x0) * frac);
                    }
                    outSpans[c].at(f) += gain_ * s;
                }
            }

            phase_ += step_;
        }


        // If we've advanced past the end (or exactly to it), the voice is finished.
        if (phase_ >= static_cast<double>(lastIndex_))
            active_ = false;
    }

    bool SamplerVoice::finished() const {
        return !active_;
    }

}
