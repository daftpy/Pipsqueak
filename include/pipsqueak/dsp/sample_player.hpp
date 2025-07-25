//
// Created by Daftpy on 7/24/2025.
//

#ifndef SAMPLE_PLAYER_HPP
#define SAMPLE_PLAYER_HPP
#include "audio_source.hpp"
#include <memory>

namespace pipsqueak::dsp {
    /**
     * @class SamplePlayer
     * @brief An audio source that plays a single audio sample from an AudioBuffer.
     */
    class SamplePlayer final : public AudioSource {
    public:
        /**
         * @brief Constructs a player with a shared pointer to the audio data.
         * @param sampleData A shared pointer to the AudioBuffer to be played.
         */
        explicit SamplePlayer(std::shared_ptr<core::AudioBuffer> sampleData);

        /**
         * @brief Starts playback from the beginning of the sample.
         */
        void play();

        /**
         * @brief Stops playback and resets the position to the beginning.
         */
        void stop();

        /**
         * @brief Sets the playback position to a new frame index.
         * @param newPosition The new position, in frames.
         */
        void setPosition(size_t newPosition);

        /**
         * @brief Gets the current playback position.
         * @return The current position, in frames.
         */
        size_t getPosition() const;

        /**
         * @brief Renders the next block of audio into the output buffer.
         * @param buffer The buffer to mix audio into.
         */
        void process(core::AudioBuffer& buffer) override;

        /**
         * @brief Checks if the player is currently inactive.
         * @return True if not playing, false otherwise.
         */
        bool isFinished() const override;

    private:
        // The shared audio data this player will read from.
        std::shared_ptr<core::AudioBuffer> sampleData_;

        // The current playback position, measured in frames.
        size_t readPosition_{0};

        // Tracks whether the player is currently active.
        bool isPlaying_{false};
    };

}
#endif //SAMPLE_PLAYER_HPP
