//
// Created by Daftpy on 7/26/2025.
//

#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <RtAudio.h>
#include <memory>

#include "pipsqueak/dsp/audio_source.hpp"

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

        // Temporary home for adding audio sources
        void addSource(std::shared_ptr<dsp::AudioSource> source);

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

        // A simple placeholder mixer of all sound sources
        std::vector<std::shared_ptr<dsp::AudioSource>> sources_;

        // Temp reusable buffer to avoid real-time allocation in the callback
        std::unique_ptr<core::AudioBuffer> mixerBuffer_{nullptr};

       // Atomic pointer for the real time audio thread to read from
       std::shared_ptr<const std::vector<std::shared_ptr<dsp::AudioSource>>> activeSources_;
    };
}

#endif //ENGINE_HPP
