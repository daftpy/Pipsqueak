########################
# --- CMake Config --- #
########################
cmake_minimum_required(VERSION 3.28.1)

# Version 0.1.0 development build
project(pipsqueak VERSION 0.1.0 LANGUAGES CXX)

# pipsqueak uses and requires the c++ 17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable FetchContent
include(FetchContent)

########################
# --- Dependencies --- #
########################

# --- RtAudio ---
FetchContent_Declare(
        RtAudio
        GIT_REPOSITORY https://github.com/thestk/rtaudio.git
        GIT_TAG        6.0.1
)
# Configure RtAudio to be built as a static library
set(RTAUDIO_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(RtAudio)

############################
# --- Pipsqueak Target --- #
############################

# Add the pipsqueak library target
add_library(pipsqueak STATIC
        src/core/audio_buffer.cpp
        include/pipsqueak/audio_io/device_scanner.hpp
        src/audio_io/device_scanner.cpp
        include/pipsqueak/engine/engine.hpp
        src/engine/engine.cpp
        include/pipsqueak/core/logging.hpp
        include/pipsqueak/audio_io/types.hpp
        include/pipsqueak/core/buffer_store.hpp
        src/core/buffer_store.cpp
        include/pipsqueak/dsp/mixer.hpp
        src/dsp/mixer.cpp
        include/pipsqueak/dsp/sampler.hpp
        include/pipsqueak/dsp/sampler_voice.hpp
        src/dsp/sampler_voice.cpp
        src/dsp/sampler.cpp
)

target_include_directories(pipsqueak
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>        # For consumers: #include <pipsqueak/audio/...>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include/pipsqueak                 # For internal use: #include "audio/engine.hpp"
)

# Link pipsqueak to its dependencies
target_link_libraries(pipsqueak
    PUBLIC
        rtaudio
)

############################
# --- Optional Testing --- #
############################
option(PIPSQUEAK_BUILD_TESTING "Build the pipsqueak tests" OFF)

if (PIPSQUEAK_BUILD_TESTING)
    # Enable the CTest framework
    include(CTest)

    # Add the tests directory
    add_subdirectory(tests)
endif ()
