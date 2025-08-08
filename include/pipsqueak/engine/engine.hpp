//
// Created by Daftpy on 7/26/2025.
//

#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <RtAudio.h>
#include <memory>

#include "pipsqueak/dsp/audio_source.hpp"
#include "pipsqueak/dsp/mixer.hpp"

namespace pipsqueak::engine {
    /**
     * @class AudioEngine
     * @brief The central class that manages the audio stream, mixing, and processing.
     */
    class AudioEngine {
    public:
        /**
         * @brief Constructs the engine and initializes the underlying audio API.
         */
        AudioEngine();

        /**
         * @brief Destructor that ensures the audio stream is safely stopped and closed.
         */
        ~AudioEngine();

        /**
         * @brief Opens and starts the audio stream with the specified device and parameters.
         * @param deviceId The ID of the audio device to use.
         * @param sampleRate The desired sample rate (e.g., 44100).
         * @param bufferSize The desired buffer size in frames.
         * @return True if the stream started successfully, false otherwise.
         */
        bool startStream(unsigned int deviceId, unsigned int sampleRate, unsigned int bufferSize);

        /**
         * @brief Stops and closes the currently active audio stream.
         * @return True if the stream was stopped successfully, false otherwise.
         */
        void stopStream();

        /**
         * @brief Checks if the audio stream is currently running.
         */
        [[nodiscard]] bool isRunning() const;

        /**
         * @brief Gets a reference to the underlying RtAudio instance for querying.
         */
        RtAudio& audio();

        /**
         * @brief Gets a reference to the engine's master mixer.
         * @return A reference to the master Mixer instance.
         */
        dsp::Mixer& masterMixer();

    private:
        /**
         * @brief The static C-style callback function passed to RtAudio.
         * Acts as a bridge to the processBlock member function.
         */
        static int audioCallback(void *outputBuffer, void * /*inputBuffer*/,
            unsigned int nFrames, double /*streamTime*/, RtAudioStreamStatus status,
            void *userData);

        /**
         * @brief The main audio processing function called by the audio thread.
         * This is where all mixing and processing occurs.
         */
        int processBlock(void* outputBuffer, unsigned int numFrames);

        // The unique_ptr manages the lifetime of the RtAudio object.
        std::unique_ptr<RtAudio> audio_;

        // A reusable buffer to avoid real-time allocation in the audio callback.
        std::unique_ptr<core::AudioBuffer> mixerBuffer_{nullptr};

        // The master mixer; the single entry point for all audio to be rendered.
        dsp::Mixer masterMixer_;
    };
}

#endif //ENGINE_HPP
