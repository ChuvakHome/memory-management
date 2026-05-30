#ifndef ALLOCATOR_THREAD_LOCAL_ALLOCATOR_HPP
#define ALLOCATOR_THREAD_LOCAL_ALLOCATOR_HPP

#include "pool_allocator.hpp"

template<class T>
class BumpPointerThreadLocalAllocator: public BasePoolAllocator<T> {
public:
    BumpPointerThreadLocalAllocator(std::size_t pool_size, std::size_t block_size = sizeof(T))
        : BasePoolAllocator<T>(pool_size, block_size)
        , first_free_(BasePoolAllocator<T>::pool_region_start_ + BasePoolAllocator<T>::pool_size_) {

    }

    BumpPointerThreadLocalAllocator(const BumpPointerThreadLocalAllocator &) = delete;

    ~BumpPointerThreadLocalAllocator() = default;

    BumpPointerThreadLocalAllocator& operator=(const BumpPointerThreadLocalAllocator &) = delete;

    T* allocate() {
        return static_cast<T *>(static_cast<void *>(first_free_ -= sizeof(T)));
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
    std::byte *first_free_;
};

#endif
