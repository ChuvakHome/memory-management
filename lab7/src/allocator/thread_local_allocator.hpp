#ifndef ALLOCATOR_THREAD_LOCAL_ALLOCATOR_HPP
#define ALLOCATOR_THREAD_LOCAL_ALLOCATOR_HPP

#include "pool_allocator.hpp"

template<class T>
class BumpPointerThreadLocalAllocator: public BasePoolAllocator<T> {
public:
    BumpPointerThreadLocalAllocator(std::size_t pool_size, std::size_t block_size = sizeof(T))
        : BasePoolAllocator<T>(pool_size, block_size)
        , first_free_(BasePoolAllocator<T>::pool_start_) {

    }

    BumpPointerThreadLocalAllocator(const BumpPointerThreadLocalAllocator &) = delete;

    ~BumpPointerThreadLocalAllocator() override = default;

    BumpPointerThreadLocalAllocator& operator=(const BumpPointerThreadLocalAllocator &) = delete;

    T* allocate() override {
        return static_cast<T *>(static_cast<void *>(first_free_ -= sizeof(T)));
    }

    void deallocate(T *ptr) {

    }
private:
    std::byte *first_free_;
};

#endif
