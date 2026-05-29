#include "pool_registry.hpp"

#include <array>
#include <atomic>
#include <cstdio>
#include <cstdlib>

namespace {
    constexpr std::size_t MAX_POOL_COUNT = 20;

    std::array<std::atomic<std::byte *>, MAX_POOL_COUNT> memory_region_starts;
    std::array<std::atomic<std::size_t>, MAX_POOL_COUNT> memory_region_sizes;
}

std::size_t pool_registry::find_pool(const std::byte *addr) {
    std::size_t i;

    for (std::size_t i = 0; i < MAX_POOL_COUNT; ++i) {
        auto reg_start = memory_region_starts[i].load(std::memory_order_relaxed);
        auto reg_end = memory_region_sizes[i].load(std::memory_order_relaxed) + reg_start;

        if (reg_start <= addr && addr < reg_end) {
            return i;
        }
    }

    return pool_registry::npos;
}

pool_registry::region_id_t pool_registry::add_memory_protected_region(MemoryRegion region) {
    for (std::size_t i = 0; i < MAX_POOL_COUNT; ++i) {
        std::byte *expected = nullptr;

        if (memory_region_starts[i].compare_exchange_strong(expected, region.start, std::memory_order_relaxed)) {
            memory_region_sizes[i].store(region.size, std::memory_order_relaxed);

            return i;
        }
    }

    std::perror("Pool collection exhaused");
    std::exit(EXIT_FAILURE);
}

void pool_registry::remove_memory_protected_region(pool_registry::region_id_t id) {
    memory_region_starts[id].store(nullptr, std::memory_order_release);
}
