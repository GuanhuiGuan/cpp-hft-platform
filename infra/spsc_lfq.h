#pragma once

#include "common.h"

#include <vector>
#include <atomic>

namespace infra {
    /* single-producer single-consumer lock-free queue */
    template<typename T>
    class SpscQueue {
    public:
        explicit SpscQueue(size_t cap) {
            ASSERT(cap > 0 && (cap & (cap-1)) == 0, "Capacity must be power of 2");
            data_ = std::vector<T>(cap, T());
            mask_ = cap - 1;
        }
        SpscQueue(const SpscQueue&) = delete;
        SpscQueue(SpscQueue&&) = delete;
        SpscQueue& operator=(const SpscQueue&) = delete;
        SpscQueue& operator=(SpscQueue&&) = delete;

        auto enqueue(T&& x) {
            size_t w = write_.load(std::memory_order_relaxed);
            data_[w] = std::move(x);
            write_.store(mod(w + 1), std::memory_order_relaxed);
            ++size_;
        }

        auto dequeue(T& x) {
            if (size() == 0) return false;
            size_t r = read_.load(std::memory_order_relaxed);
            x = data_[r];
            read_.store(mod(r + 1), std::memory_order_relaxed);
            --size_;
            return true;
        }

        auto atWrite() const {return &data_[write_.load(std::memory_order_relaxed)];}
        auto atRead() const {return size() > 0 ? &data_[read_.load(std::memory_order_relaxed)] : nullptr;}

        // if write_ laps read_, then size_ won't be correct
        inline size_t size() const {return size_.load(std::memory_order_relaxed);}

    private:
        std::vector<T> data_;
        size_t mask_;
        std::atomic_size_t size_{0};
        std::atomic_size_t read_{0};
        std::atomic_size_t write_{0};

        inline size_t mod(size_t idx) const {
            // return idx % data_.size();
            return idx & mask_;
        }
    };
}