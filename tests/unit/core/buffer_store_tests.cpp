//
// Created by Daftpy on 8/6/2025.
//

#include <gtest/gtest.h>

#include <pipsqueak/core/buffer_store.hpp>
#include <pipsqueak/core/audio_buffer.hpp>

// A test fixture to set up a BufferStore for each test
class BufferStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a new BufferStore with a capacity of 10 before each test
        store = std::make_unique<pipsqueak::core::BufferStore>(200);
    }

    std::unique_ptr<pipsqueak::core::BufferStore> store;
};

// Test that we can insert a buffer and retrieve it successfully.
TEST_F(BufferStoreTest, InsertAndGet) {
    // Create a sample audio buffer (2 channels, 441 frames)
    const auto buffer = std::make_shared<pipsqueak::core::AudioBuffer>(2, 441);

    // Insert the buffer and get its unique key
    const size_t key = store->insert(buffer);

    // Retrieve the buffer using the key
    auto retrievedBuffer = store->get(key);

    // Assert that the retrieved buffer is not null and is the same as the original
    ASSERT_NE(retrievedBuffer, nullptr);
    ASSERT_EQ(retrievedBuffer, buffer);
    ASSERT_EQ(retrievedBuffer->numChannels(), 2);
    ASSERT_EQ(retrievedBuffer->numFrames(), 441);
}

// Test that getting a non-existent key returns a nullptr.
TEST_F(BufferStoreTest, GetNonExistentReturnsNull) {
    // Attempt to get a buffer with a key that hasn't been inserted
    const auto retrievedBuffer = store->get(999);
    ASSERT_EQ(retrievedBuffer, nullptr);
}

// Test that we can successfully erase a buffer.
TEST_F(BufferStoreTest, EraseExisting) {
    const auto buffer = std::make_shared<pipsqueak::core::AudioBuffer>(1, 100);
    const size_t key = store->insert(buffer);

    // Erase the buffer and assert that the operation was successful
    ASSERT_TRUE(store->erase(key));

    // Verify that the buffer is no longer in the store
    auto retrievedBuffer = store->get(key);
    ASSERT_EQ(retrievedBuffer, nullptr);
}

// Test that attempting to erase a non-existent key fails gracefully.
TEST_F(BufferStoreTest, EraseNonExistent) {
    // Attempting to erase a key that doesn't exist should return false
    ASSERT_FALSE(store->erase(999));
}

// Test that consecutively inserted buffers receive unique, incrementing IDs.
TEST_F(BufferStoreTest, InsertGeneratesUniqueIDs) {
    auto buffer1 = std::make_shared<pipsqueak::core::AudioBuffer>(1, 1);
    auto buffer2 = std::make_shared<pipsqueak::core::AudioBuffer>(1, 1);

    const size_t key1 = store->insert(buffer1);
    const size_t key2 = store->insert(buffer2);

    // Assert that the keys are unique and increment as expected (0, 1, 2...)
    ASSERT_NE(key1, key2);
    ASSERT_EQ(key1, 0);
    ASSERT_EQ(key2, 1);
}

// Test that the store can handle concurrent insertions from multiple threads.
TEST_F(BufferStoreTest, ConcurrentInsertsAreThreadSafe) {
    constexpr int numThreads = 100;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    // This lambda function will be executed by each thread.
    auto insert_task = [&]() {
        const auto buffer = std::make_shared<pipsqueak::core::AudioBuffer>(1, 1);
        store->insert(buffer);
    };

    // Launch all the threads. They will all try to execute the task concurrently.
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(insert_task);
    }

    // Wait for all threads to complete their work.
    for (auto& t : threads) {
        t.join();
    }

    // After all threads are done, check the state of the store.
    // 100 unique keys have have been inserted successfully.
    // This code runs without crashing or corrupting data.
    const auto finalBuffer = std::make_shared<pipsqueak::core::AudioBuffer>(1, 1);

    // Verify that all insertions were successful by checking the final ID.
    const size_t finalKey = store->insert(finalBuffer);
    ASSERT_EQ(finalKey, numThreads);
}