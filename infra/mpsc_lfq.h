#pragma once

#include "common.h"

#include <vector>
#include <atomic>

namespace infra {

    template<typename T>
    class MpscQueue {
    public:
        explicit MpscQueue(size_t cap) {
            ASSERT(cap > 0 && (cap & (cap-1)) == 0, "Capacity should be power of 2");
            mask_ = cap - 1;
            data_ = std::vector<T>(cap, T());
        }
        MpscQueue(const MpscQueue&) = delete;
        MpscQueue(MpscQueue&&) = delete;
        MpscQueue& operator=(const MpscQueue&) = delete;
        MpscQueue& operator=(MpscQueue&&) = delete;

        auto dequeue(long& x) {
            if (empty()) return false;
            size_t r = read_.load(std::memory_order_relaxed);
            x = data_[r];
            read_.store(mod(r + 1), std::memory_order_relaxed);
            size_.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }

        auto enqueue(T&& x) {
            size_t w = write_.load(std::memory_order_acquire);
            while (!write_.compare_exchange_weak(w, mod(w + 1), std::memory_order_release));
            data_[w] = std::move(x);
            size_.fetch_add(1, std::memory_order_seq_cst);
        }

        bool empty() const {return size() == 0;}
        size_t size() const {return size_.load(std::memory_order_seq_cst);}
    private:
        std::vector<T> data_;
        size_t mask_;
        std::atomic_size_t size_{0};
        std::atomic_size_t write_{0};
        std::atomic_size_t read_{0};

        size_t mod(size_t idx) const {return idx & mask_;}
    };
}