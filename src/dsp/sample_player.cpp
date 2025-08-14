//
// Created by Daftpy on 7/25/2025.
//

#include "dsp/sample_player.hpp"
#include "core/logging.hpp"
#include <pipsqueak/core/channel_view.hpp>

namespace pipsqueak::dsp {
    SamplePlayer::SamplePlayer(std::shared_ptr<const core::AudioBuffer> sampleData)
    : sampleData_(std::move(sampleData)) {
        core::logging::Logger::log("pipsqueak", "SamplePlayer initialized!");
    }

    void SamplePlayer::play() {
        isPlaying_ = true;
        readPosition_ = 0;
    }

    void SamplePlayer::stop() {
        isPlaying_ = false;
        readPosition_ = 0;
    }

    bool SamplePlayer::isFinished() const {
        return !isPlaying_;
    }

    void SamplePlayer::process(core::AudioBuffer& buffer) {
    // If not playing or no sample data, do nothing
    if (!isPlaying_ || !sampleData_) {
        return;
    }

    const unsigned int outCh = buffer.numChannels();
    const unsigned int srcCh = sampleData_->numChannels();

    // If we've reached the end of the source, stop.
    const size_t srcFrames = sampleData_->numFrames();
    if (readPosition_ >= srcFrames) {
        isPlaying_ = false;
        return;
    }

    // Only render what we have left in the source (don’t overrun).
    const size_t framesToRender =
        std::min(static_cast<size_t>(buffer.numFrames()), srcFrames - readPosition_);

    if (framesToRender == 0) {
        // Nothing to render in this callback.
        return;
    }

    if (srcCh == 1) {
        // Mono source: duplicate to all output channels.
        const auto src = sampleData_->channel(0).raw(); // RawSpan<true>

        // Prepare writable spans for output channels once.
        std::vector<decltype(buffer.channel(0).raw())> outs;
        outs.reserve(outCh);
        for (unsigned int c = 0; c < outCh; ++c) {
            outs.push_back(buffer.channel(c).raw());     // RawSpan<false>
        }

        const size_t srcStart = readPosition_;
        for (size_t f = 0; f < framesToRender; ++f) {
            const core::Sample s = src.at(srcStart + f);
            for (unsigned int c = 0; c < outCh; ++c) {
                outs[c].at(f) += s;
            }
        }
    } else {
        // Stereo / multi-channel: copy channel-for-channel.
        const unsigned int n = std::min(outCh, srcCh);

        std::vector<decltype(sampleData_->channel(0).raw())> srcs;
        srcs.reserve(n);
        for (unsigned int c = 0; c < n; ++c) {
            srcs.push_back(sampleData_->channel(c).raw());  // RawSpan<true>
        }

        std::vector<decltype(buffer.channel(0).raw())> outs;
        outs.reserve(n);
        for (unsigned int c = 0; c < n; ++c) {
            outs.push_back(buffer.channel(c).raw());        // RawSpan<false>
        }

        const size_t srcStart = readPosition_;
        for (size_t f = 0; f < framesToRender; ++f) {
            for (unsigned int c = 0; c < n; ++c) {
                outs[c].at(f) += srcs[c].at(srcStart + f);
            }
        }
    }

    // Advance read position and update state.
    readPosition_ += framesToRender;
    if (readPosition_ >= srcFrames) {
        isPlaying_ = false;
    }
}


    void SamplePlayer::setPosition(const size_t newPosition) {
        readPosition_ = newPosition;
    }

    size_t SamplePlayer::getPosition() const {
        return readPosition_;
    }

}