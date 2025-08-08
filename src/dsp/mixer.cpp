//
// Created by Daftpy on 8/8/2025.
//

#include <algorithm>
#include <pipsqueak/dsp/mixer.hpp>

namespace pipsqueak::dsp {
    Mixer::Mixer() {
        // Initialize with a valid, empty list of sources to ensure thread safety from the start.
        const auto initialSources = std::make_shared<const std::vector<std::shared_ptr<AudioSource>>>();
        std::atomic_store(&activeSources_, initialSources);
    }

    void Mixer::addSource(std::shared_ptr<AudioSource> source) {
        // Atomically get a snapshot of the current list of sources.
        const auto currentSources = std::atomic_load(&activeSources_);
        // Create a new, mutable list by copying the current one.
        auto mutableSources = std::make_shared<std::vector<std::shared_ptr<AudioSource>>>(*currentSources);
        // Add the new source to our private, mutable copy.
        mutableSources->push_back(std::move(source));
        // Create a final, read-only shared_ptr that matches the member variable's type.
        const std::shared_ptr<const std::vector<std::shared_ptr<AudioSource>>> finalSources = mutableSources;
        // Atomically swap the active pointer to make the new list live for the audio thread.
        std::atomic_store(&activeSources_, finalSources);
    }

    void Mixer::clearSources() {
        // Create a new, completely empty list of sources.
        const auto emptySources = std::make_shared<const std::vector<std::shared_ptr<AudioSource>>>();
        // Atomically swap the active pointer to point to the new empty list.
        std::atomic_store(&activeSources_, emptySources);
    }

    bool Mixer::isFinished() const {
        // Atomically get the current list of sources.
        const auto currentSources = std::atomic_load(&activeSources_);
        // A mixer is considered finished if and only if all of its sources are finished.
        return std::all_of(currentSources->begin(), currentSources->end(),
                           [](const auto& source) { return source->isFinished(); });
    }

    void Mixer::process(core::AudioBuffer& buffer) {
        // Atomically get the list of sources to process for this specific audio block.
        const auto sourcesToProcess = std::atomic_load(&activeSources_);

        // Process each source, mixing (adding) its output into the provided buffer.
        for (const auto& source : *sourcesToProcess) {
            source->process(buffer);
        }
    }
}