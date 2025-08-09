# pipsqueak

[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)](https://isocpp.org/)

A minimal, modern C++17 audio engine designed as a foundational toolkit for building real-time audio applications like DAWs, trackers, and games.

## Features

* **Modern C++17 Design:** Utilizes modern C++ features for safe, expressive, and efficient code.
* **Real-Time Safe Mixing:** Employs a lock-free, atomic swap pattern for adding and removing audio sources, preventing audio glitches and dropouts in multi-threaded environments.
* **Composable Audio Graph:** Built on a modular `AudioSource` interface where a `Mixer` is also an `AudioSource`, allowing for powerful and complex routing (e.g., submixes).
* **Clean Hardware Abstraction:** The low-level `AudioEngine` is cleanly separated from the DSP components, providing a simple interface to the physical audio hardware.
* **Ergonomic Data Handling:** Provides a powerful `AudioBuffer` class and a lightweight `ChannelView` for intuitive, zero-overhead manipulation of audio channels.

---

## Core Concepts

Pipsqueak is built on a few simple, powerful ideas:

* **`engine::AudioEngine`**: The hardware abstraction layer. Its only job is to communicate with the system's audio device via RtAudio. It owns the master `Mixer` and drives the audio processing.
* **`dsp::AudioSource`**: An interface for any object that can produce audio. This is the contract that makes the engine modular.
* **`dsp::Mixer`**: A thread-safe `AudioSource` that acts as a summing bus. It takes other `AudioSource`s as input, mixes their output, and is the main entry point for all audio into the `AudioEngine`.
* **`dsp::SamplePlayer`**: A simple `AudioSource` for playing back audio data held in an `AudioBuffer`.
* **`core::AudioBuffer`**: The central data container for blocks of high-precision, interleaved audio samples.
* **`core::ChannelView`**: A lightweight, non-owning view for accessing a single channel of an `AudioBuffer` in a non-interleaved way.
* **`audio_io::DeviceScanner`**: A utility for querying the system for available audio hardware.

---

## Getting Started

The following example shows how to initialize the engine, generate a simple sine wave, and play it.

```cpp
#include <iostream>
#include <cmath>
#include <pipsqueak/engine/engine.hpp>
#include <pipsqueak/dsp/sample_player.hpp>
#include <pipsqueak/audio_io/device_scanner.hpp>

constexpr double PI = 3.14159265358979323846;

int main() {
    // 1. Create the engine.
    pipsqueak::engine::AudioEngine engine;
    
    // 2. Find and start the audio stream.
    pipsqueak::audio_io::DeviceScanner scanner(engine.audio());
    if (auto defaultDevice = scanner.defaultDevice()) {
        engine.startStream(defaultDevice->ID, 44100, 512);
    } else {
        std::cerr << "No audio device found!\n";
        return -1;
    }

    // 3. Create some audio data (a 1-second, 440 Hz sine wave).
    const unsigned int sampleRate = 44100;
    auto sineBuffer = std::make_shared<pipsqueak::core::AudioBuffer>(1, sampleRate);
    for (unsigned int i = 0; i < sineBuffer->numFrames(); ++i) {
        sineBuffer->at(0, i) = 0.5 * std::sin(2.0 * PI * 440.0 * i / sampleRate);
    }
    
    // 4. Create a player for our sine wave.
    auto player = std::make_shared<pipsqueak::dsp::SamplePlayer>(sineBuffer);
    
    // 5. Add the player to the engine's master mixer and play it.
    engine.masterMixer().addSource(player);
    player->play();

    std::cout << "Playing sine wave... Press Enter to quit.\n";
    std::cin.get();
    
    // 6. Stop the stream.
    engine.stopStream();
    
    return 0;
}
```

## Building

Pipsqueak uses CMake for building.

**Requirements:**

- A C++17 compatible compiler (MSVC, GCC, Clang)
- CMake (3.15 or higher)

```bash
# Clone the repository
git clone [https://github.com/daftpy/pipsqueak.git](https://github.com/daftpy/pipsqueak.git)
cd pipsqueak

# Configure the build
cmake -B build

# Build the project
cmake --build build
```

## Dependencies
- **RtAudio**: A cross-platform audio I/O library. It is fetched and built automatically by CMake via `FetchContent`.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

For information on third-party libraries used by this project, please see the [THIRD-PARTY-LICENSES.txt](THIRD-PARTY-LICENSES.txt) file.