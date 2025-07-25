#ifndef CHANNEL_VIEW_HPP
#define CHANNEL_VIEW_HPP

#include "audio_buffer.hpp"

namespace pipsqueak::core {

    /**
     * @class ChannelView
     * @brief A lightweight view into a single channel of an AudioBuffer.
     * @tparam BufferType The type of buffer to view (either AudioBuffer or const AudioBuffer).
     */
    template <typename BufferType>
    class ChannelView {
    public:
        /**
         * @brief Constructs a view for a specific channel of a buffer.
         */
        ChannelView(BufferType* buffer, const unsigned int channelIndex)
            : buffer_(buffer), channelIndex_(channelIndex) {}

        /**
         * @brief Provides read-write access to a sample by its frame index.
         * @note This method is only available for non-const (writable) views.
         */
        double& operator[](unsigned int frameIndex) {
            return const_cast<double&>(
                static_cast<const ChannelView&>(*this)[frameIndex]
            );
        }

        /**
         * @brief Provides read-only access to a sample by its frame index.
         */
        const double& operator[](const unsigned int frameIndex) const {
            return buffer_->at(channelIndex_, frameIndex);
        };

        /**
         * @brief Applies a gain factor to every sample in this channel.
         * @note This method is only available for non-const (writable) views.
         */
        void applyGain(double gainFactor) {
            if constexpr (std::is_const_v<std::remove_pointer_t<BufferType>> == false) {
                for (size_t i = 0; i < size(); ++i) {
                    (*this)[i] *= gainFactor;
                }
            }
        };

        /**
         * @brief Fills every sample in this channel with a specific value.
         * @note This method is only available for non-const (writable) views.
         */
        void fill(double value) {
            if constexpr (std::is_const_v<std::remove_pointer_t<BufferType>> == false) {
                for (size_t i = 0; i < size(); ++i) {
                    (*this)[i] = value;
                }
            }
        };

        /**
         * @brief Returns the number of frames (samples) in this channel view.
         */
        [[nodiscard]] size_t size() const {
            return buffer_->numFrames();
        };

        /**
         * @brief Copies samples from a source range into this channel.
         * @tparam InputIter The type of the iterator for the source data.
         * @param first An iterator to the beginning of the source data.
         * @param last An iterator to the end of the source data.
         * @note This method is only available for non-const (writable) views.
         */
        template <typename InputIter>
        void copyFrom(InputIter first, InputIter last) {
            // This method should only work on a writable view.
            if constexpr (!std::is_const_v<std::remove_pointer_t<BufferType>>) {
                const auto numToCopy = std::min(
                    static_cast<size_t>(std::distance(first, last)),
                    size()
                );

                for (size_t i = 0; i < numToCopy; ++i) {
                    (*this)[i] = *first++;
                }
            }
        }

    private:
        // Pointer to the AudioBuffer. Either AudioBuffer* or const AudioBuffer*.
        BufferType* buffer_;
        // The index of the channel this view represents.
        unsigned int channelIndex_;
    };

    // --- Type Aliases ---
    /// A read-write view of a single audio channel.
    class WritableChannelView : public ChannelView<AudioBuffer> {
        using ChannelView<AudioBuffer>::ChannelView; // Inherit constructor
    };

    /// A read-only view of a single audio channel.
    class ReadOnlyChannelView : public ChannelView<const AudioBuffer> {
        using ChannelView<const AudioBuffer>::ChannelView; // Inherit constructor
    };
}

#endif //CHANNEL_VIEW_HPP