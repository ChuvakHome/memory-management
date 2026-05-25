
#ifndef ALLOCATOR_POOL_ALLOCATOR_HPP
#define ALLOCATOR_POOL_ALLOCATOR_HPP

#include <algorithm>

#include <cassert>
#include <cstddef>

#include <sys/mman.h>

#include <unistd.h>
#include <utility>

namespace detail {
    const std::size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);

    inline std::size_t align_size(std::size_t sz, std::size_t alignment) {
        assert((alignment & (alignment - 1)) == 0); // align should be a power of 2

        const std::size_t rem = sz & (alignment - 1);
        return (sz & ~(alignment - 1)) + std::clamp<std::size_t>(rem, 0, 1) * alignment;
    }
}

template<class T>
class BumpPointerAllocator {
public:
    BumpPointerAllocator(std::size_t pool_size, std::size_t block_size = sizeof(T)) {
        assert(pool_size > 0);
        assert(block_size > 0);

        std::size_t actual_pool_size = detail::align_size(pool_size, detail::PAGE_SIZE);
        std::size_t aligned_block_size = detail::align_size(block_size, detail::PAGE_SIZE);

        pool_size_ = actual_pool_size + aligned_block_size,
        first_free_ = static_cast<std::byte *>(mmap(
            nullptr,
            pool_size_,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_PRIVATE,
            -1,
            0
        ));

        if (first_free_ == MAP_FAILED) {
            perror("Initial memory allocation failed");

            std::exit(EXIT_FAILURE);
        }

        if (mprotect(first_free_, aligned_block_size, PROT_NONE)) {
            perror("Unable to protect first page");

            std::exit(EXIT_FAILURE);
        }

        first_free_ += pool_size_;
    }

    BumpPointerAllocator(const BumpPointerAllocator &) = delete;

    ~BumpPointerAllocator() {
        free_pool();
    }

    BumpPointerAllocator& operator=(const BumpPointerAllocator &) = delete;

    T* allocate() {
        return static_cast<T*>(static_cast<void *>(first_free_ -= sizeof(T)));
    }

    template<class... Ts>
    T* construct(Ts&&... args) {
        T* instance = allocate();
        new(instance) T{std::forward<Ts>(args)...};

        return instance;
    }

    void deallocate(T *ptr, std::size_t n = 1) {

    }

    void free_pool() {
        if (first_free_ != nullptr) {
            munmap(first_free_, pool_size_);
            first_free_ = nullptr;
        }
    }
private:
    std::size_t pool_size_ = 0;
    std::byte *first_free_ = nullptr;
};

#endif
