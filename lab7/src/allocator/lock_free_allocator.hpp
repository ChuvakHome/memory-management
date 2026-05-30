
#ifndef ALLOCATOR_LOCK_FREE_ALLOCATOR_HPP
#define ALLOCATOR_LOCK_FREE_ALLOCATOR_HPP

#include "pool_allocator.hpp"
#include <atomic>
#include <cstddef>

template<class T>
class BumpPointerLockFreeAllocator: public BasePoolAllocator<T> {
public:
    BumpPointerLockFreeAllocator(std::size_t pool_size, std::size_t block_size = sizeof(T))
        : BasePoolAllocator<T>(pool_size, block_size)
        , first_free_(BasePoolAllocator<T>::pool_region_start_ + BasePoolAllocator<T>::pool_size_) {

    }

    BumpPointerLockFreeAllocator(const BumpPointerLockFreeAllocator &) = delete;

    T* allocate() {
        return static_cast<T*>(
            static_cast<void *>(
                first_free_.fetch_sub(static_cast<std::ptrdiff_t>(sizeof(T)), std::memory_order_relaxed) - sizeof(T)
            )
        );
    }

    template<class... Ts>
    T* construct(Ts&&... args) {
        T* instance = allocate();
        new(instance) T{std::forward<Ts>(args)...};

        return instance;
    }

    void deallocate(T *ptr) {

    }
private:
    std::atomic<std::byte *> first_free_;
};

#endif
