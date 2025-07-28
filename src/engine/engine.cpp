//
// Created by Daftpy on 7/26/2025.
//
#include "pipsqueak/engine/engine.hpp"
#include "pipsqueak/core/logging.hpp"

namespace pipsqueak::engine {
    int AudioEngine::audioCallback(void *outputBuffer, void * /*inputBuffer*/,
        const unsigned int nFrames, double /*streamTime*/,
        const RtAudioStreamStatus status, void *userData) {

        if (status)
            std::cerr << "Stream underflow or overflow detected!\n";

        // If the cast is successful, process the audio
        if (auto* engine = static_cast<AudioEngine*>(userData)) {
            return engine->processBlock(outputBuffer, nFrames);
        }

        return 0;
    }

    AudioEngine::AudioEngine() : audio_(std::make_unique<RtAudio>()) {
        core::logging::Logger::log("pipsqueak", "AudioEngine initialized!");
        // std::cout << "AudioEngine - status running: " << isRunning() << "\n";
    }

    AudioEngine::~AudioEngine() {
        core::logging::Logger::log("pipsqueak", "AudioEngine destroyed!");
    }

    int AudioEngine::processBlock(void* outputBuffer, unsigned int numFrames) {
        // 1. Clear the buffer to silence
        mixerBuffer_->fill(0.0);

        // 2. Process all active audio sources, mixing them into the buffer.
        for (const auto& source : sources_) {
            source->process(*mixerBuffer_);
        }

        // 3. TODO: process a master effect chain

        // 4. Copy the final mixed audio to the hardware output buffer.
        double* hardwareBuffer = static_cast<double*>(outputBuffer);
        std::copy(mixerBuffer_->data().begin(), mixerBuffer_->data().end(), hardwareBuffer);

        return 0;
    }

    bool AudioEngine::startStream(unsigned int deviceId, unsigned int sampleRate, unsigned int bufferSize) {
        core::logging::Logger::log("pipsqueak", "starting stream (sample rate: " +
            std::to_string(sampleRate) + " | buffer: " + std::to_string(bufferSize) + ")");
        const RtAudio::DeviceInfo info = audio_->getDeviceInfo(deviceId);

        // Set the output parameters
        RtAudio::StreamParameters outputParams;
        outputParams.deviceId = deviceId;
        outputParams.firstChannel = 0;
        outputParams.nChannels = info.outputChannels;

        unsigned int negotiatedBufferSize = bufferSize;

        // Try to open the stream
        if (const auto err = audio_->openStream(
            &outputParams, nullptr, RTAUDIO_FLOAT32,
            sampleRate, &negotiatedBufferSize, &AudioEngine::audioCallback, nullptr
        ); err != RTAUDIO_NO_ERROR) {
            std::cerr << "AudioEngine failed to open stream: " << audio_->getErrorText() << "\n";
            return false;
        }

        // Create the mixer buffer with the appropriate size
        mixerBuffer_ = std::make_unique<core::AudioBuffer>(outputParams.nChannels, negotiatedBufferSize);

        // Try to start the stream
        if (const auto err = audio_->startStream(); err != RTAUDIO_NO_ERROR) {
            std::cerr << "AudioEngine failed to start stream: " << audio_->getErrorText() << "\n";
            return false;
        }

        core::logging::Logger::log("pipsqueak", "AudioEngine stream started successfully!");
        return true;
    }

    void AudioEngine::stopStream() {
        if (!isRunning())
            return;

        if (audio_->isStreamRunning()) {
            if (const auto err = audio_->stopStream(); err != RTAUDIO_NO_ERROR)
                std::cerr << "AudioEngine failed to stop the stream: "
                << audio_->getErrorText() << "\n";
        }

        if (audio_->isStreamOpen())
            audio_->closeStream();

        core::logging::Logger::log("pipsqueak", "AudioEngine has stopped the stream!");
    }

    bool AudioEngine::isRunning() const {
        return audio_->isStreamRunning();
    }

    RtAudio& AudioEngine::audio() {
        return *audio_;
    }

    void AudioEngine::addSource(std::shared_ptr<dsp::AudioSource> source) {
        // TODO: implement adding multiple sound sources to be mixed down into the stream
    }
}