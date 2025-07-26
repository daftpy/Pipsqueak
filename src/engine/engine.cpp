//
// Created by Daftpy on 7/26/2025.
//
#include "pipsqueak/engine/engine.hpp"

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
        std::cout << "AudioEngine initialized!\n";
        // std::cout << "AudioEngine - status running: " << isRunning() << "\n";
    }

    AudioEngine::~AudioEngine() {
        std::cout << "AudioEngine - status running: " << isRunning() << "\n";
        std::cout << "AudioEngine destroyed!\n";
    }

    int AudioEngine::processBlock(void* outputBuffer, unsigned int numFrames) {
        // TODO: Process audio
        return 0;
    }

    bool AudioEngine::startStream(unsigned int deviceId, unsigned int sampleRate, unsigned int bufferSize) {
        std::cout << "AudioEngine - starting stream\n";
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

        std::cout << "AudioEngine stream started successfully!\n";
        return true;
    }

    bool AudioEngine::stopStream() {
        if (!isRunning())
            return false;

        if (audio_->isStreamRunning()) {
            if (const auto err = audio_->stopStream(); err != RTAUDIO_NO_ERROR)
                std::cerr << "AudioEngine failed to stop stream: "
                << audio_->getErrorText() << "\n";
            return false;
        }

        if (audio_->isStreamOpen())
            audio_->closeStream();

        std::cout << "AudioEngine has stopped the stream\n";
        return true;
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