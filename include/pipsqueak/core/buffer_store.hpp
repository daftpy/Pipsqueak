//
// Created by Daftpy on 8/6/2025.
//

#ifndef BUFFER_STORE_HPP
#define BUFFER_STORE_HPP

#include <memory>
#include <unordered_map>
#include <shared_mutex>

#include "audio_buffer.hpp"

namespace pipsqueak::core {
    class BufferStore {
    public:
        explicit BufferStore(size_t capacity);
        ~BufferStore() = default;

        size_t insert(std::shared_ptr<const AudioBuffer> buffer);

        std::shared_ptr<const AudioBuffer> get(size_t key);

        bool erase(size_t key);

    private:
        size_t capacity_;
        size_t ID_{0};

        mutable std::shared_mutex mutex_;
        std::unordered_map<size_t, std::shared_ptr<const AudioBuffer>> cache_;
    };
}

#endif //BUFFER_STORE_HPP
