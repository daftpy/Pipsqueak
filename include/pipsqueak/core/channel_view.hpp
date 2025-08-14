#ifndef CHANNEL_VIEW_HPP
#define CHANNEL_VIEW_HPP

#include "audio_buffer.hpp"
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <cstddef>

namespace pipsqueak::core {

    /**
     * @class ChannelView
     * @brief Lightweight view into a single channel of an interleaved AudioBuffer.
     *
     * @tparam BufferType Either @c AudioBuffer (writable view) or @c const AudioBuffer (read-only view).
     *
     * Provides two access styles:
     * - Safe, bounds-checked element access via @c operator[] which internally calls @c AudioBuffer::at().
     * - Zero-overhead, strided access via @c raw() returning a (ptr, frames, stride) "span" for tight DSP loops.
     *
     * Also exposes iterators (@c begin/@c end and @c cbegin/@c cend) that walk the channel
     * frame-by-frame using the interleave stride.
     */
    template <typename BufferType>
    class ChannelView {
    public:
        /// True if this view is writable (i.e., @p BufferType is non-const).
        static constexpr bool Writable = !std::is_const_v<BufferType>;

        /**
         * @brief Construct a view for a specific channel of a buffer.
         * @param buffer Pointer to the parent buffer (constness matches @p BufferType).
         * @param channelIndex Channel index in [0, buffer->numChannels()).
         * @warning The caller must ensure @p channelIndex is valid; creation does not check bounds.
         */
        ChannelView(BufferType* buffer, const unsigned int channelIndex)
            : buffer_(buffer), channelIndex_(channelIndex) {}

        /**
         * @brief Writable element access by frame index.
         * @details Enabled only when @c Writable is true. Uses the buffer's bounds-checked access.
         * @param frameIndex Frame index in [0, size()).
         * @return Reference to the writable sample.
         * @throws std::out_of_range if @p frameIndex is out of bounds (thrown by @c AudioBuffer::at()).
         */
        template <typename T = BufferType, typename = std::enable_if_t<Writable>>
        Sample& operator[](size_t frameIndex) {
            return const_cast<Sample&>(static_cast<const ChannelView&>(*this)[frameIndex]);
        }

        /**
         * @brief Read-only element access by frame index.
         * @details Always available. Uses the buffer's bounds-checked access.
         * @param frameIndex Frame index in [0, size()).
         * @return Const reference to the sample.
         * @throws std::out_of_range if @p frameIndex is out of bounds (thrown by @c AudioBuffer::at()).
         */
        const Sample& operator[](const size_t frameIndex) const {
            return buffer_->at(channelIndex_, frameIndex);
        }

        /**
         * @brief Apply a gain factor to every sample in this channel.
         * @details Enabled only when @c Writable is true. Runs a single pass over frames.
         * @param gainFactor Linear gain multiplier (cast to @c Sample).
         */
        void applyGain(const double gainFactor) {
            if constexpr (Writable) {
                const auto g = static_cast<Sample>(gainFactor);
                for (size_t i = 0; i < size(); ++i) {
                    (*this)[i] *= g;
                }
            }
        }

        /**
         * @brief Fill the channel with a constant value.
         * @details Enabled only when @c Writable is true. Runs a single pass over frames.
         * @param value Fill value (cast to @c Sample).
         */
        void fill(const double value) {
            if constexpr (Writable) {
                const auto v = static_cast<Sample>(value);
                for (size_t i = 0; i < size(); ++i) {
                    (*this)[i] = v;
                }
            }
        }

        /**
         * @brief Number of frames (samples) in this channel view.
         * @return Frame count (equals parent buffer's @c numFrames()).
         */
        [[nodiscard]] size_t size() const { return buffer_->numFrames(); }

        /**
         * @brief Strided, zero-overhead view of this channel's samples.
         * @tparam Const When true, exposes a @c const Sample*; otherwise @c Sample*.
         *
         * Represents a channel over interleaved storage using (ptr, frames, stride).
         * The element at frame @c i is located at @c ptr[i * stride].
         * Intended for hot DSP loops — no bounds checks are performed.
         */
        template <bool Const>
        struct RawSpan {
            /// Pointer to the first sample of this channel (channel offset already applied).
            using Ptr = std::conditional_t<Const, const Sample*, Sample*>;
            /// Reference type for element access via @c at().
            using Ref = std::conditional_t<Const, const Sample&, Sample&>;

            Ptr   ptr;      ///< Base pointer to channel data.
            size_t frames;  ///< Number of frames available.
            size_t stride;  ///< Interleave stride (== parent buffer's numChannels()).

            /**
             * @brief Unchecked element access by frame index.
             * @param i Frame index in [0, frames).
             * @return Reference to @c ptr[i * stride].
             * @warning No bounds checks are performed.
             */
            Ref at(size_t i) const noexcept { return *(ptr + i * stride); }
        };

        /**
         * @brief Return a writable raw span for fast DSP on this channel.
         * @details Enabled only when @c Writable is true. Uses unchecked pointer+stride access.
         * @return @c RawSpan<false> with @c Sample* pointer.
         */
        template <typename T = BufferType, typename = std::enable_if_t<Writable>>
        auto raw() noexcept -> RawSpan<false> {
            return { buffer_->dataPtr() + channelIndex_,
                     buffer_->numFrames(),
                     static_cast<size_t>(buffer_->interleaveStride()) };
        }

        /**
         * @brief Return a read-only raw span for fast DSP on this channel.
         * @details Always available. Uses unchecked pointer+stride access.
         * @return @c RawSpan<true> with @c const Sample* pointer.
         */
        [[nodiscard]] auto raw() const noexcept -> RawSpan<true> {
            return { buffer_->dataPtr() + channelIndex_,
                     buffer_->numFrames(),
                     static_cast<size_t>(buffer_->interleaveStride()) };
        }

        /**
         * @brief Copy from a source range into this channel.
         * @tparam InputIter Forward/RandomAccess iterator over values convertible to @c Sample.
         * @param first Iterator to beginning of source range.
         * @param last  Iterator to end of source range.
         * @details Enabled only when @c Writable is true. Copies up to @c size() elements;
         *          extra source elements are ignored.
         */
        template <typename InputIter>
        void copyFrom(InputIter first, InputIter last) {
            if constexpr (Writable) {
                const auto n = std::min(static_cast<size_t>(std::distance(first, last)), size());
                for (size_t i = 0; i < n; ++i) {
                    (*this)[i] = *first++;
                }
            }
        }

        // ----------------- Iteration support (frame-wise, strided) -----------------

        /**
         * @brief Forward iterator over frames (writable variant).
         * @tparam Const When true, iterator yields @c const Sample&, else @c Sample&.
         *
         * The iterator advances one frame at a time using the channel's interleave stride.
         */
        template <bool Const>
        class StridedIterator {
        public:
            using Ptr               = std::conditional_t<Const, const Sample*, Sample*>;
            using Ref               = std::conditional_t<Const, const Sample&, Sample&>;
            using difference_type   = std::ptrdiff_t;
            using value_type        = Sample;
            using reference         = Ref;
            using pointer           = Ptr;
            using iterator_category = std::forward_iterator_tag;

            StridedIterator(Ptr base, size_t idx, size_t stride)
                : base_(base), idx_(idx), stride_(stride) {}

            reference operator*() const noexcept { return *(base_ + idx_ * stride_); }
            StridedIterator& operator++() noexcept { ++idx_; return *this; }
            bool operator==(const StridedIterator& other) const noexcept {
                return base_ == other.base_ && idx_ == other.idx_ && stride_ == other.stride_;
            }
            bool operator!=(const StridedIterator& other) const noexcept { return !(*this == other); }

        private:
            Ptr   base_;
            size_t idx_;
            size_t stride_;
        };

        /**
         * @brief Begin iterator over frames (writable view).
         * @return Iterator to the first frame (enabled only when @c Writable is true).
         */
        template <typename T = BufferType, typename = std::enable_if_t<Writable>>
        auto begin() noexcept -> StridedIterator<false> {
            auto s = raw();
            return { s.ptr, 0, s.stride };
        }

        /**
         * @brief End iterator over frames (writable view).
         * @return Iterator past the last frame (enabled only when @c Writable is true).
         */
        template <typename T = BufferType, typename = std::enable_if_t<Writable>>
        auto end() noexcept -> StridedIterator<false> {
            auto s = raw();
            return { s.ptr, s.frames, s.stride };
        }

        /**
         * @brief Begin iterator over frames (const view).
         * @return Const iterator to the first frame.
         */
        auto begin() const noexcept -> StridedIterator<true> {
            auto s = raw();
            return { s.ptr, 0, s.stride };
        }

        /**
         * @brief End iterator over frames (const view).
         * @return Const iterator past the last frame.
         */
        auto end() const noexcept -> StridedIterator<true> {
            auto s = raw();
            return { s.ptr, s.frames, s.stride };
        }

        /**
         * @brief Const begin iterator alias.
         */
        auto cbegin() const noexcept -> StridedIterator<true> { return begin(); }

        /**
         * @brief Const end iterator alias.
         */
        auto cend() const noexcept -> StridedIterator<true> { return end(); }

    private:
        BufferType*   buffer_;       ///< Pointer to the parent AudioBuffer (constness matches @p BufferType).
        unsigned int  channelIndex_; ///< Channel index this view represents.
    };

    // --- Type Aliases ---

    /// Writable view of a single audio channel.
    class WritableChannelView : public ChannelView<AudioBuffer> {
        using ChannelView<AudioBuffer>::ChannelView; // inherit constructor
    };

    /// Read-only view of a single audio channel.
    class ReadOnlyChannelView : public ChannelView<const AudioBuffer> {
        using ChannelView<const AudioBuffer>::ChannelView; // inherit constructor
    };

} // namespace pipsqueak::core

#endif // CHANNEL_VIEW_HPP
