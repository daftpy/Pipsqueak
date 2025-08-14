#include "core/audio_buffer.hpp"
#include "core/channel_view.hpp"
#include "core/logging.hpp"
#include <stdexcept>
#include <string>

namespace pipsqueak::core {

    AudioBuffer::AudioBuffer(const unsigned int numChannels, const unsigned int numFrames)
        : numChannels_(numChannels), numFrames_(numFrames), data_(static_cast<size_t>(numChannels) * static_cast<size_t>(numFrames)) {
        const std::string message = "AudioBuffer initialized! Channels: " + std::to_string(numChannels) + ", Frames: "
            + std::to_string(numFrames);
        logging::Logger::log("pipsqueak", message);
    }

    unsigned int AudioBuffer::numChannels() const {
        return numChannels_;
    }

    unsigned int AudioBuffer::numFrames() const {
        return numFrames_;
    }

    const PCMData& AudioBuffer::data() const {
        return data_;
    }

    PCMData& AudioBuffer::data() {
        return data_;
    }

    // This is the primary implementation of the at() method, with bounds checking.
    const Sample& AudioBuffer::at(const unsigned int channelNum, const unsigned int frameNum) const {
        if (channelNum >= numChannels_ || frameNum >= numFrames_) {
            throw std::out_of_range(
                "AudioBuffer access out of range."
                "Accessed [ch:" + std::to_string(channelNum) + ", fr:" + std::to_string(frameNum) + "], "
                "but size is [ch:" + std::to_string(numChannels_) + ", fr:" + std::to_string(numFrames_) + "]."
            );
        }
        // Get the index as size_t
        const size_t idx = static_cast<size_t>(frameNum) * numChannels_ + channelNum;
        return data_[idx];
    }

    // Reuses the const version's logic to avoid code duplication.
    Sample& AudioBuffer::at(const unsigned int channelNum, const unsigned int frameNum) {
        return const_cast<Sample&>(static_cast<const AudioBuffer&>(*this).at(channelNum, frameNum));
    }

    // Factory method to create a view for the specified channel.
    WritableChannelView AudioBuffer::channel(const unsigned int channelNum) {
        if (channelNum >= numChannels_) {
            throw std::out_of_range("Invalid channel index provided to channel().");
        }
        return WritableChannelView{this, channelNum};
    }

    // Factory method to create a read-only view for a specified channel.
    ReadOnlyChannelView AudioBuffer::channel(const unsigned int channelNum) const {
        if (channelNum >= numChannels_) {
            throw std::out_of_range("Invalid channel index provided to channel().");
        }
        return ReadOnlyChannelView{this, channelNum};
    }

    // Applies the gain factor to all channels in the buffer.
    void AudioBuffer::applyGain(const double gainFactor) {
        for (unsigned int i{0}; i < numChannels_; ++i) {
            channel(i).applyGain(gainFactor);
        }
    }

    // Sets all samples in the buffer to a given value.
    void AudioBuffer::fill(const double value) {
        for (unsigned int i{0}; i < numChannels_; ++i) {
            channel(i).fill(value);
        }
    }
}