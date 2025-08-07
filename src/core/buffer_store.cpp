//
// Created by Daftpy on 8/6/2025.
//

#include "pipsqueak/core/buffer_store.hpp"

#include "core/logging.hpp"

namespace pipsqueak::core {
    BufferStore::BufferStore(const size_t capacity) : capacity_(capacity) {
        logging::Logger::log(
            "pipsqueak", "AudioStore initialized. Capacity - " + std::to_string(capacity_)
        );
    }

    size_t BufferStore::insert(std::shared_ptr<const AudioBuffer> buffer) {
        std::unique_lock lock(mutex_);

        // Get the new ID and move the buffer
        const size_t ID = ID_++;
        cache_[ID] = std::move(buffer);

        return ID;
    }

    std::shared_ptr<const AudioBuffer> BufferStore::get(const size_t key) {
        std::shared_lock lock(mutex_);

        // Find and return the buffer
        if (const auto it = cache_.find(key); it != cache_.end()) {
            return it->second;
        }

        return nullptr;
    }

    bool BufferStore::erase(const size_t key) {
        std::unique_lock lock(mutex_);

        // Delete the buffer if found
        if (const auto it = cache_.find(key); it != cache_.end()) {
            cache_.erase(it);
            return true;
        }
        return false;
    }
}
