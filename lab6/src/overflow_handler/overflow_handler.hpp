
#ifndef OVERFLOW_HANDLER_OVERFLOW_HANDLER_HPP
#define OVERFLOW_HANDLER_OVERFLOW_HANDLER_HPP

#include <cstddef>

struct MemoryRegion {
    std::byte *start;
    std::size_t size;

    std::byte* get_end() const {
        return start + size;
    }

    bool contains(std::byte *addr) const {
        return start <= addr && addr <= get_end();
    }
};

void add_overflow_handler();

void add_memory_protected_region(MemoryRegion region);

void remove_memory_protected_region();

#endif
