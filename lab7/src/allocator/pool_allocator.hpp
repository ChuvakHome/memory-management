
#ifndef ALLOCATOR_ALLOCATOR_HPP
#define ALLOCATOR_ALLOCATOR_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>

#include <sys/mman.h>
#include <unistd.h>

#include "pool_registry.hpp"

namespace detail {
    const std::size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);

    inline std::size_t align_size(std::size_t sz, std::size_t alignment) {
        assert((alignment & (alignment - 1)) == 0); // align should be a power of 2

        const std::size_t rem = sz & (alignment - 1);
        return (sz & ~(alignment - 1)) + std::clamp<std::size_t>(rem, 0, 1) * alignment;
    }
}


template<class T>
class BasePoolAllocator {
public:
    BasePoolAllocator(std::size_t pool_size, std::size_t block_size) {
        assert(pool_size > 0);
        assert(block_size > 0);

        std::size_t actual_pool_size = detail::align_size(pool_size, detail::PAGE_SIZE);
        std::size_t aligned_block_size = detail::align_size(block_size, detail::PAGE_SIZE);

        pool_size_ = actual_pool_size + aligned_block_size,
        pool_start_ = static_cast<std::byte *>(mmap(
            nullptr,
            pool_size_,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_PRIVATE,
            -1,
            0
        ));

        if (pool_start_ == MAP_FAILED) {
            perror("Initial memory allocation failed");

            std::exit(EXIT_FAILURE);
        }

        prot_start_ = pool_start_;
        prot_size_ = aligned_block_size;

        if (mprotect(prot_start_, prot_size_, PROT_NONE)) {
            perror("Unable to protect first page");

            std::exit(EXIT_FAILURE);
        }

        region_id_ = pool_registry::add_memory_protected_region({prot_start_, prot_size_});

        pool_start_ += pool_size_;
    }

    virtual ~BasePoolAllocator() {
        free_pool();
    }

    virtual T* allocate() = 0;

    template<class... Ts>
    T* construct(Ts&&... args) {
        T* instance = allocate();
        new(instance) T{std::forward<Ts>(args)...};

        return instance;
    }

    void free_pool() {
        if (pool_start_ != nullptr) {
            munmap(prot_start_, pool_size_);
            pool_registry::remove_memory_protected_region(region_id_);

            prot_start_ = nullptr;
            prot_size_ = 0;

            pool_start_ = nullptr;
            pool_size_ = 0;
        }
    }
protected:
    std::size_t pool_size_ = 0;
    std::byte *pool_start_ = nullptr;
private:
    pool_registry::region_id_t region_id_;

    std::byte *prot_start_ = nullptr;
    std::size_t prot_size_ = 0;
};

#endif
