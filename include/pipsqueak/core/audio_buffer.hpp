//
// Created by Daftpy on 7/24/2025.
//

#ifndef AUDIO_BUFFER_HPP
#define AUDIO_BUFFER_HPP

#include "types.hpp"

namespace pipsqueak::core {
    // Forward declaration
    class WritableChannelView;
    class ReadOnlyChannelView;

    /**
     * @class AudioBuffer
     * @brief A container for multi-channel, interleaved audio data.
     * Manages the memory and provides safe access to audio samples.
     */
    class AudioBuffer {
    public:
        /**
         * @brief Constructs a buffer with the given dimensions, allocating memory.
         * @param numChannels The number of channels (e.g., 2 for stereo).
         * @param numFrames The number of sample frames (the length of the buffer).
         */
        AudioBuffer(unsigned int numChannels, unsigned numFrames);

        /**
         * @brief Gets the number of audio channels in the buffer.
         */
        unsigned int numChannels() const;

        /**
         * @brief Gets the number of sample frames (i.e., the length) of the buffer.
         */
        unsigned int numFrames() const;

       /**
        * @brief Returns a read-write view for a single channel.
        */
        WritableChannelView channel(unsigned int channelNum);

        ReadOnlyChannelView channel(unsigned int channelNum) const;

        /**
         * @brief Provides direct access to the raw interleaved sample data.
         * @note Provided for high-performance algorithms that need to operate on the whole buffer.
         */
        [[nodiscard]] const PCMData& data() const;
        PCMData& data();

        /**
         * @brief Provides safe, bounds-checked access to an individual sample.
         * @param channelNum The channel of the sample to access.
         * @param frameNum The frame of the sample to access.
         * @return A reference to the sample.
         * @throws std::out_of_range if access is out of bounds.
         */
        const double& at(unsigned int channelNum, unsigned int frameNum) const;
        double& at(unsigned int channelNum, unsigned int frameNum);

        /**
         * @brief Applies a gain factor to all samples in the buffer.
         * @param gainFactor The gain multiplier.
         */
        void applyGain(double gainFactor);

        /**
         * @brief Sets all samples in the buffer to a given fill value.
         * @param value The fill value.
         */
        void fill(double value);


       /**
        * @brief Copies interleaved sample data from a source range into this buffer.
        * @tparam InputIter The type of the iterator for the source data.
        * @param first An iterator to the beginning of the source data.
        * @param last An iterator to the end of the source data.
        */
        template <typename InputIter>
        void copyFrom(InputIter first, InputIter last) {
            const auto sourceSize = static_cast<size_t>(std::distance(first, last));
            const auto numToCopy = std::min(sourceSize, data_.size());

            std::copy(first, first + numToCopy, data_.begin());
        }

    private:
        // The dimensions of the audio buffer.
        unsigned int numChannels_;
        unsigned int numFrames_;

        // The raw sample data, stored in an interleaved format (e.g., L, R, L, R).
        PCMData data_;
    };
}

#endif //AUDIO_BUFFER_HPP
