// Created by Daftpy on 7/25/2025.

#include <gtest/gtest.h>
#include <pipsqueak/dsp/sampler.hpp>
#include <pipsqueak/core/audio_buffer.hpp>
#include <pipsqueak/core/channel_view.hpp>
#include <memory>
#include <vector>

// Helper to create a buffer
static std::shared_ptr<pipsqueak::core::AudioBuffer>
makeBuffer(unsigned int channels, unsigned int frames) {
    return std::make_shared<pipsqueak::core::AudioBuffer>(channels, frames);
}

// Helper: configure rates BEFORE noteOn for deterministic tests
static void setRates(pipsqueak::dsp::Sampler& s, double rate) {
    s.setNativeRate(rate);
    s.setEngineRate(rate);
}

// --- Tests ---

// Newly created Sampler has no active voices.
TEST(SamplerTest, InitialStateIsInactive) {
    auto buf = makeBuffer(1, 100);
    pipsqueak::dsp::Sampler sampler(buf);
    EXPECT_TRUE(sampler.isFinished());
}

// Inactive Sampler must not modify the buffer (mixer-style contract: add only).
TEST(SamplerTest, InactiveSamplerDoesNotModifyBuffer) {
    auto buf = makeBuffer(1, 100);
    pipsqueak::dsp::Sampler sampler(buf);

    pipsqueak::core::AudioBuffer out(2, 256);
    out.fill(0.5); // non-zero sentinel

    const std::vector<float> original = out.data();
    sampler.process(out);
    EXPECT_EQ(out.data(), original);
}

// noteOn makes it active and it writes samples.
TEST(SamplerTest, NoteOnActivatesAndWrites) {
    auto sample = makeBuffer(1, 256);
    sample->fill(0.77);

    pipsqueak::dsp::Sampler sampler(sample);
    setRates(sampler, 48000.0);

    ASSERT_TRUE(sampler.isFinished()); // before noteOn
    sampler.noteOn(48, 1.0f);          // root note, full velocity
    ASSERT_FALSE(sampler.isFinished());

    pipsqueak::core::AudioBuffer out(2, 64);
    out.fill(0.0);
    sampler.process(out);

    // Should have written ~0.77 to both channels (mono → stereo duplicate).
    for (unsigned f = 0; f < out.numFrames(); ++f) {
        EXPECT_NEAR(out.at(0, f), 0.77, 1e-6);
        EXPECT_NEAR(out.at(1, f), 0.77, 1e-6);
    }
}

// Mono source copied to both channels of a stereo output.
TEST(SamplerTest, ProcessCopiesMonoSourceToStereoOutput) {
    auto sample = makeBuffer(1, 512);
    sample->fill(0.25);

    pipsqueak::dsp::Sampler sampler(sample);
    setRates(sampler, 48000.0);
    sampler.noteOn(48, 1.0f);

    pipsqueak::core::AudioBuffer out(2, 256);
    out.fill(0.0);
    sampler.process(out);

    for (unsigned f = 0; f < out.numFrames(); ++f) {
        EXPECT_NEAR(out.at(0, f), 0.25, 1e-6);
        EXPECT_NEAR(out.at(1, f), 0.25, 1e-6);
    }
}

// Stereo source preserved per channel.
TEST(SamplerTest, ProcessCopiesStereoSourceToStereoOutput) {
    auto sample = makeBuffer(2, 512);
    sample->channel(0).fill(0.5);
    sample->channel(1).fill(-0.5);

    pipsqueak::dsp::Sampler sampler(sample);
    setRates(sampler, 48000.0);
    sampler.noteOn(48, 1.0f);

    pipsqueak::core::AudioBuffer out(2, 256);
    out.fill(0.0);
    sampler.process(out);

    for (unsigned f = 0; f < out.numFrames(); ++f) {
        EXPECT_NEAR(out.at(0, f),  0.5, 1e-6);
        EXPECT_NEAR(out.at(1, f), -0.5, 1e-6);
    }
}

// While noteOff isn’t implemented, rendering past the end should finish the voice.
TEST(SamplerTest, FinishesAfterEndOfSample) {
    auto sample = makeBuffer(1, 64);
    sample->fill(1.0);

    pipsqueak::dsp::Sampler sampler(sample);
    setRates(sampler, 48000.0);
    sampler.noteOn(48, 1.0f);

    // Render more frames than the sample has to force completion.
    pipsqueak::core::AudioBuffer out(1, 128);
    out.fill(0.0);
    sampler.process(out);

    EXPECT_TRUE(sampler.isFinished());
}
