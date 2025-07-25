//
// Created by Daftpy on 7/25/2025.
//

#include "dsp/sample_player.hpp"

#include <iostream>

namespace pipsqueak::dsp {
    SamplePlayer::SamplePlayer(std::shared_ptr<core::AudioBuffer> sampleData)
    : sampleData_(std::move(sampleData)) {
        std::cout << "SamplePlayer initialized!\n";
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

        const auto numOutputChannels = buffer.numChannels();
        const auto numSourceChannels = sampleData_->numChannels();

        // Loop through each frame of the output buffer.
        for (unsigned int f{0}; f < buffer.numFrames(); ++f) {
            // Check if we're at the end of the sample data
            if (readPosition_ >= sampleData_->numFrames()) {
                isPlaying_ = false;
                break; // Exit the loop
            }

            // Copy the data for the current frame
            if (numSourceChannels == 1) {
                // Mono sources copy the single sample to all output channels
                const auto sampleValue = sampleData_->at(0, readPosition_);
                for (unsigned int c{0}; c < numOutputChannels; ++c) {
                    // Mix the data with the buffer
                    buffer.at(c, f) += sampleValue;
                }
            } else {
                // Stereo/multi-channel sources copy channel for channel.
                const auto numChannelsToCopy = std::min(numSourceChannels, numOutputChannels);
                for (unsigned int c{0}; c < numChannelsToCopy; ++c) {
                    buffer.at(c, f) += sampleData_->at(c, readPosition_);
                }
            }

            // Advanced the read position
            readPosition_++;
        }
    }

    void SamplePlayer::setPosition(const size_t newPosition) {
        readPosition_ = newPosition;
    }

    size_t SamplePlayer::getPosition() const {
        return readPosition_;
    }

}