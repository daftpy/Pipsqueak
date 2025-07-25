# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
)

# Force Google Test and its dependencies to be built as static libraries
set(BUILD_SHARED_LIBS OFF)
# Ensure Google Test uses the same C++ runtime as the main project
set(gtest_force_shared_crt OFF)

# Make the source available
FetchContent_MakeAvailable(googletest)

# Define the test executable
add_executable(pipsqueak_test
        core/audio_buffer_tests.cpp
        dsp/sample_player_tests.cpp
)

target_link_libraries(pipsqueak_test
    gtest_main
    pipsqueak
)

include(GoogleTest)
gtest_discover_tests(pipsqueak_test
        DISCOVERY_MODE PRE_TEST
)