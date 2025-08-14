//
// Created by Daftpy on 7/24/2025.
//

#ifndef AUDIO_BUFFER_HPP
#define AUDIO_BUFFER_HPP

#include "logging.hpp"
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
        AudioBuffer(unsigned int numChannels, unsigned int numFrames);

        /**
         * @brief Constructs and populates a buffer from existing interleaved sample data.
         * @details Copies and converts from @p initialData into internal storage.
         *          If @p initialData is nullptr, the buffer is **zero-filled** (policy choice)
         *          and construction succeeds.
         * @tparam SampleType Numeric type of the source data (e.g., float, double, int16_t).
         * @param numChannels Number of channels in the source data.
         * @param numFrames   Number of frames in the source data.
         * @param initialData Pointer to interleaved source data (may be nullptr to zero-fill).
         */
        template<typename SampleType>
        AudioBuffer(const unsigned int numChannels, const unsigned int numFrames, const SampleType* initialData)
            : numChannels_(numChannels),
              numFrames_(numFrames),
              data_(static_cast<size_t>(numChannels) * static_cast<size_t>(numFrames)) // Pre-allocate the vector
        {
            if (!initialData) {
             // Policy: zero-fill when no initial data is provided.
             std::fill(data_.begin(), data_.end(), static_cast<Sample>(0));
             logging::Logger::log("pipsqueak",
                 "AudioBuffer: null initialData; zero-filled buffer");
                 return;
            }

            // Convert & copy from interleaved source
            const size_t total = static_cast<size_t>(numChannels_) * static_cast<size_t>(numFrames_);
            for (size_t i = 0; i < total; ++i) {
                data_[i] = static_cast<Sample>(initialData[i]);
            }
        }

        /**
         * @brief Gets the number of audio channels in the buffer.
         */
        [[nodiscard]] unsigned int numChannels() const;

        /**
         * @brief Gets the number of sample frames (i.e., the length) of the buffer.
         */
        [[nodiscard]] unsigned int numFrames() const;

       /**
        * @brief Returns a read-write view for a single channel.
        */
        WritableChannelView channel(unsigned int channelNum);

        [[nodiscard]] ReadOnlyChannelView channel(unsigned int channelNum) const;

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
        [[nodiscard]] const Sample& at(unsigned int channelNum, unsigned int frameNum) const;
        Sample& at(unsigned int channelNum, unsigned int frameNum);

        /**
         * @brief Unchecked element access (no bounds checks).
         * @param channelNum Channel index in [0, numChannels()).
         * @param frameNum   Frame index in [0, numFrames()).
         * @return Reference to the sample.
         * @warning Caller must ensure indices are valid. Use only in hot paths when already validated.
         */
        [[nodiscard]] const Sample& at_unchecked(unsigned int channelNum, unsigned int frameNum) const noexcept;
        Sample& at_unchecked(unsigned int channelNum, unsigned int frameNum) noexcept;

        /**
         * @brief Returns a raw pointer to the interleaved sample data.
         * @return Pointer to @c Sample storage (size = numChannels() * numFrames()).
         * @note Pointer remains valid for the lifetime of the AudioBuffer object
         *       and until any operation that may reallocate the underlying vector.
         */
        [[nodiscard]] Sample* dataPtr() noexcept;
        [[nodiscard]] const Sample* dataPtr() const noexcept;

        /**
         * @brief Returns the interleave stride for the data layout.
         * @details This equals the number of channels and is the increment (in samples)
         *          to move from frame @c i to frame @c i+1 for the same channel.
         * @return Interleave stride (== numChannels()).
         */
        [[nodiscard]] unsigned int interleaveStride() const noexcept;

        /**
         * @brief Applies a gain factor to all samples in the buffer.
         * @details Single-pass implementation over interleaved storage.
         *          @p gainFactor is cast to @c Sample before multiplication.
         * @param gainFactor Linear gain multiplier (double, cast to Sample).
         */
        void applyGain(double gainFactor);

        /**
         * @brief Sets all samples in the buffer to a given value.
         * @details Single-pass implementation over interleaved storage.
         *          @p value is cast to @c Sample before assignment.
         * @param value Fill value (double, cast to Sample).
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
