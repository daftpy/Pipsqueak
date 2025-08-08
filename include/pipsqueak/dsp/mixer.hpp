//
// Created by Daftpy on 8/8/2025.
//

#ifndef MIXER_HPP
#define MIXER_HPP

#include "audio_source.hpp"
#include <memory>
#include <vector>
#include <atomic>

namespace pipsqueak::dsp {
    /**
     * @class Mixer
     * @brief An AudioSource that mixes the output of multiple other AudioSources.
     * @details Acts as a summing bus, allowing multiple sounds to be played
     * simultaneously. This class is thread-safe for adding and removing sources.
     */
    class Mixer final : public AudioSource {
    public:
        /**
         * @brief Constructs an empty Mixer.
         */
        Mixer();

        /**
         * @brief Thread-safely adds a new audio source to the mixer.
         * @param source The source to add.
         */
        void addSource(std::shared_ptr<AudioSource> source);

        /**
         * @brief Thread-safely removes all audio sources from the mixer.
         */
        void clearSources();

        /**
         * @brief Renders audio by summing the output of all contained sources.
         * @param buffer The output buffer to mix audio into.
         */
        void process(core::AudioBuffer& buffer) override;

        /**
         * @brief Checks if the mixer is finished.
         * @return True if all contained sources are finished, false otherwise.
         */
        [[nodiscard]] bool isFinished() const override;

    private:
        // A thread-safe pointer to the read-only list of sources for the audio thread.
        std::shared_ptr<const std::vector<std::shared_ptr<AudioSource>>> activeSources_;
    };
}

#endif //MIXER_HPP