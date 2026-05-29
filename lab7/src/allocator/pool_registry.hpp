
#ifndef ALLOCATOR_POOL_REGISTRY_HPP
#define ALLOCATOR_POOL_REGISTRY_HPP

#include <cstddef>

namespace pool_registry {
    struct MemoryRegion {
        std::byte * start;
        std::size_t size;
    };

    using region_id_t = std::size_t;

    constexpr std::size_t npos = -1;

    std::size_t find_pool(const std::byte *addr);

    region_id_t add_memory_protected_region(MemoryRegion region);

    void remove_memory_protected_region(region_id_t);
}

#endif
