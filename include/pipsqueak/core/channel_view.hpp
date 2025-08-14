#ifndef CHANNEL_VIEW_HPP
#define CHANNEL_VIEW_HPP

#include "audio_buffer.hpp"
#include <algorithm>
#include <type_traits>
#include <iterator>

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
         * @details Enabled only for writable views (i.e., when BufferType is non-const).
         * Uses the buffer's bounds-checked access internally.
         * @param frameIndex Frame index in [0, size()).
         * @return Reference to the writable sample in this channel at the given frame.
         * @throws std::out_of_range if frameIndex is out of bounds.
         */
        template <typename T = BufferType, typename = std::enable_if_t<!std::is_const_v<T>>>
        Sample& operator[](size_t frameIndex) {
            return const_cast<Sample&>(static_cast<const ChannelView&>(*this)[frameIndex]);
        }

        /**
         * @brief Provides read-only access to a sample by its frame index.
         * @details Always available. Uses the buffer's bounds-checked access internally.
         * @param frameIndex Frame index in [0, size()).
         * @return Const reference to the sample in this channel at the given frame.
         * @throws std::out_of_range if frameIndex is out of bounds.
         */
        const Sample& operator[](const size_t frameIndex) const {
            return buffer_->at(channelIndex_, frameIndex);
        };

        /**
         * @brief Applies a gain factor to every sample in this channel.
         * @details Enabled only for writable views. Casts @p gainFactor to @c Sample.
         * Runs in a single pass over the channel frames.
         * @param gainFactor Linear gain multiplier (double, cast to Sample).
         */
        void applyGain(const double gainFactor) {
            if constexpr (!std::is_const_v<BufferType>) {
                const auto g = static_cast<Sample>(gainFactor);
                for (size_t i = 0; i < size(); ++i) {
                    (*this)[i] *= g;
                }
            }
        };

        /**
         * @brief Fills every sample in this channel with a specific value.
         * @details Enabled only for writable views. Casts @p value to @c Sample.
         * Runs in a single pass over the channel frames.
         * @param value The fill value (double, cast to Sample).
         */
        void fill(const double value) {
            if constexpr (std::is_const_v<BufferType> == false) {
                const auto v = static_cast<Sample>(value);
                for (size_t i = 0; i < size(); ++i) {
                    (*this)[i] = v;
                }
            }
        };

        /**
         * @brief Returns the number of frames (samples) in this channel view.
         * @return Frame count for this channel (equals the parent buffer's numFrames()).
         */
        [[nodiscard]] size_t size() const {
            return buffer_->numFrames();
        };

        /**
         * @brief Strided, zero-overhead view of a single channel.
         * @tparam Const When true, exposes a @c const Sample*; otherwise @c Sample*.
         *
         * @details Represents a channel over interleaved storage using
         *          (ptr, frames, stride). The element at frame @c i is
         *          @c ptr[i * stride]. This is an unchecked fast path intended
         *          for DSP hot loops.
         */
        template <bool Const>
        struct RawSpan {
            /// Pointer to the first sample of this channel (channel offset already applied).
            using Ptr  = std::conditional_t<Const, const Sample*, Sample*>;
            /// Reference type for element access via at().
            using Ref  = std::conditional_t<Const, const Sample&, Sample&>;

            Ptr   ptr;      ///< Base pointer to channel data.
            size_t frames;  ///< Number of frames available.
            size_t stride;  ///< Interleave stride (== parent buffer's numChannels()).

            /**
             * @brief Unchecked element access by frame index.
             * @param i Frame index in [0, frames).
             * @return Reference to element @c ptr[i * stride].
             * @warning No bounds checks are performed.
             */
            Ref at(size_t i) const noexcept { return *(ptr + i * stride); }
        };

        /**
         * @brief Returns a writable raw span for fast DSP on this channel.
         * @details Enabled only for writable views (BufferType non-const).
         * The returned span uses unchecked pointer + stride access.
         * @return RawSpan<false> with @c Sample* pointer.
         */
        template <typename T = BufferType, typename = std::enable_if_t<!std::is_const_v<T>>>
        auto raw() noexcept -> RawSpan<false> {
            return { buffer_->dataPtr() + channelIndex_,
                     buffer_->numFrames(),
                     static_cast<size_t>(buffer_->interleaveStride()) };
        }

        /**
         * @brief Returns a read-only raw span for fast DSP on this channel.
         * @details Available for all views. The returned span uses unchecked pointer + stride access.
         * @return RawSpan<true> with @c const Sample* pointer.
         */
        [[nodiscard]] auto raw() const noexcept -> RawSpan<true> {
            return { buffer_->dataPtr() + channelIndex_,
                     buffer_->numFrames(),
                     static_cast<size_t>(buffer_->interleaveStride()) };
        }

        /**
         * @brief Copies samples from a source range into this channel.
         * @tparam InputIter Forward/RandomAccess iterator over values convertible to @c Sample.
         * @param first Iterator to beginning of source range.
         * @param last  Iterator to end of source range.
         * @details Enabled only for writable views. Copies up to @c size() elements;
         *          extra source elements are ignored.
         */
        template <typename InputIter>
        void copyFrom(InputIter first, InputIter last) {
            // This method should only work on a writable view.
            if constexpr (!std::is_const_v<BufferType>) {
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