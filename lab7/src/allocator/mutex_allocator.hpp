
#ifndef ALLOCATOR_BUMP_POINTER_ALLOCATOR_HPP
#define ALLOCATOR_BUMP_POINTER_ALLOCATOR_HPP

#include <mutex>

#include "pool_allocator.hpp"

template<class T>
class BumpPointerMutexAllocator: public BasePoolAllocator<T> {
public:
    BumpPointerMutexAllocator(std::size_t pool_size, std::size_t block_size = sizeof(T))
        : BasePoolAllocator<T>(pool_size, block_size)
        , first_free_(BasePoolAllocator<T>::pool_start_) {

    }

    BumpPointerMutexAllocator(const BumpPointerMutexAllocator &) = delete;

    ~BumpPointerMutexAllocator() override = default;

    BumpPointerMutexAllocator& operator=(const BumpPointerMutexAllocator &) = delete;

    T* allocate() override {
        std::scoped_lock<std::mutex> lock(mtx_);
        return static_cast<T *>(static_cast<void *>(first_free_ -= sizeof(T)));
    }

    void deallocate(T *ptr) {

    }
private:
    std::mutex mtx_;
    std::byte *first_free_;
};

#endif
