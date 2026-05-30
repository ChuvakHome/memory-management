#include "parallel_copy.hpp"

#include <cstddef>
#include <cstring>

#include <functional>
#include <latch>
#include <optional>

#include "thread_pool.hpp"

namespace {
    #ifndef COPY_THREADS_NUM
    #define COPY_THREADS_NUM 1
    #endif

    constexpr std::size_t MIN_CHUNK_SIZE = 1 << 12;

    std::optional<ThreadPool<std::function<void()>>> pool = std::nullopt;
}

void set_up_thread_pool(std::size_t threads_num) {
    pool.emplace(threads_num);
}

void* parallel_memcpy(void *dst, const void *src, std::size_t size) {
    if (size == 0) {
        return dst;
    }

    const std::byte *orig = static_cast<const std::byte *>(src);
    std::byte *dest = static_cast<std::byte *>(dst);

    const std::size_t threads_num = pool->get_threads_num();
    const std::size_t task_num = std::clamp<std::size_t>(size / MIN_CHUNK_SIZE, 0, threads_num);
    std::latch latch{std::ptrdiff_t(task_num)};

    const std::size_t chunk_size = size / (task_num + 1);
    const std::size_t remaining = chunk_size * task_num;

    for (std::size_t i = 0; i < task_num; ++i) {
        pool->submit([&latch, chunk_size, to = dest + i * chunk_size, from = orig + i * chunk_size]() {
            std::memcpy(to, from, chunk_size);
            latch.count_down();
        });
    }

    std::memcpy(dest + remaining, orig + remaining, size - remaining);

    latch.wait();
    pool->stop();

    return dst;
}
