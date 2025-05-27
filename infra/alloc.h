#pragma once

#include "macro.h"

#include <vector>

namespace infra {

    template<typename T>
    class Allocator final {

        struct Block {
            T data_;
            bool used_;
        };
        std::vector<Block> blocks_;
        size_t nextFreeIdx_;

    public:
        explicit Allocator(size_t size) : blocks_(size > 0 ? size : 1, {T(), false}), nextFreeIdx_{0} {}
        Allocator(const Allocator&) = delete;
        Allocator& operator=(const Allocator&) = delete;
        Allocator(Allocator&&) = delete;
        Allocator& operator=(Allocator&&) = delete;

        template<typename... Args>
        T* alloc(const Args&... args) {
            return alloc(std::forward<Args>(args)...);
        }

        template<typename... Args>
        T* alloc(Args&&... args) {
            if(nextFreeIdx_ < 0 || nextFreeIdx_ >= blocks_.size() || blocks_[nextFreeIdx_].used_) {
                return nullptr;
            }
            
            // create entity
            const size_t retIdx = nextFreeIdx_;
            blocks_[retIdx].used_ = true;
            // placement new
            new (&(blocks_[retIdx].data_)) T(std::forward<Args>(args)...);

            // find next free block
            updateNextFreeIdx();
            return reinterpret_cast<T*>(&blocks_[retIdx]);
        }

        void free(const T* ptr, bool destroy = false) {
            const auto idx = reinterpret_cast<const Block*>(ptr) - &blocks_[0];
            ASSERT(idx >= 0 && idx < blocks_.size(), "not from this allocator");

            // reset block
            blocks_[idx].used_ = false;
            if (destroy) blocks_[idx].data_.~T();
        }

        size_t capacity() const {return blocks_.size();}
        size_t nextIdx() const {return nextFreeIdx_;}

    private:
        auto updateNextFreeIdx() {
            const size_t startIdx = nextFreeIdx_;
            while (blocks_[nextFreeIdx_].used_) {
                ++nextFreeIdx_;
                [[unlikely]] if (nextFreeIdx_ == blocks_.size()) nextFreeIdx_ = 0;
                [[unlikely]] if (nextFreeIdx_ == startIdx) { // out of mem
                    return false;
                    // if resized, the existing ptrs will be invalid
                    // nextFreeIdx_ = blocks_.size();
                    // blocks_.resize(2 * blocks_.size());
                }
            }
            return true;
        }
    };
}