// Created by Daftpy on 8/8/2025.
#include <gtest/gtest.h>
#include <pipsqueak/dsp/mixer.hpp>
#include <pipsqueak/dsp/sampler.hpp>
#include <pipsqueak/core/audio_buffer.hpp>
#include <atomic>
#include <thread>

// Helper: make a mono buffer filled with a value
static std::shared_ptr<pipsqueak::core::AudioBuffer>
makeMonoFilled(unsigned frames, double value) {
    auto buf = std::make_shared<pipsqueak::core::AudioBuffer>(1, frames);
    buf->fill(value);
    return buf;
}

// Test that the mixer correctly sums the output of multiple samplers.
TEST(MixerTest, SumsSamplersCorrectly) {
    using namespace pipsqueak;

    dsp::Mixer mixer;
    constexpr unsigned int numFrames = 16;

    // Two sources with constant values
    auto src1 = makeMonoFilled(numFrames, 0.2);
    auto src2 = makeMonoFilled(numFrames, 0.3);

    auto s1 = std::make_shared<dsp::Sampler>(src1);
    auto s2 = std::make_shared<dsp::Sampler>(src2);

    // Keep rates equal to avoid resampling effects for this test
    s1->setNativeRate(48000.0);
    s1->setEngineRate(48000.0);
    s2->setNativeRate(48000.0);
    s2->setEngineRate(48000.0);

    // Trigger both voices at root note with full velocity
    s1->noteOn(48, 1.0f);
    s2->noteOn(48, 1.0f);

    mixer.addSource(s1);
    mixer.addSource(s2);

    core::AudioBuffer out(1, numFrames);
    out.fill(0.0);

    mixer.process(out);

    // Expect 0.2 + 0.3 = 0.5 per frame
    for (unsigned i = 0; i < numFrames; ++i) {
        EXPECT_NEAR(out.at(0, i), 0.5, 1e-9);
    }
}

// Test that after clearing the sources, the mixer produces silence.
TEST(MixerTest, ClearSourcesResultsInSilence) {
    using namespace pipsqueak;

    dsp::Mixer mixer;
    constexpr unsigned int numFrames = 16;

    auto src = makeMonoFilled(numFrames, 0.5);
    auto sampler = std::make_shared<dsp::Sampler>(src);
    sampler->setNativeRate(48000.0);
    sampler->setEngineRate(48000.0);
    sampler->noteOn(48, 1.0f);

    mixer.addSource(sampler);

    core::AudioBuffer out(1, numFrames);
    out.fill(0.0);

    // Clear then process
    mixer.clearSources();
    mixer.process(out);

    for (unsigned i = 0; i < numFrames; ++i) {
        EXPECT_NEAR(out.at(0, i), 0.0, 1e-9);
    }
}

// Stress test: writer adds/clears samplers while reader processes.
TEST(MixerTest, ConcurrentReadWriteIsSafe) {
    using namespace pipsqueak;

    dsp::Mixer mixer;
    core::AudioBuffer out(1, 16);
    std::atomic<bool> stop = false;

    // Writer simulates a dispatcher
    std::thread writer([&](){
        while (!stop.load(std::memory_order_relaxed)) {
            auto src = makeMonoFilled(16, 0.1);
            auto sampler = std::make_shared<dsp::Sampler>(src);
            sampler->setNativeRate(48000.0);
            sampler->setEngineRate(48000.0);
            sampler->noteOn(48, 1.0f);

            mixer.addSource(sampler);
            mixer.clearSources();
        }
    });

    // Reader simulates the audio thread
    std::thread reader([&](){
        while (!stop.load(std::memory_order_relaxed)) {
            mixer.process(out);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    stop = true;
    writer.join();
    reader.join();

    SUCCEED();
}
