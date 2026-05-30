
#ifndef THREAD_POOL_PARALLEL_COPY_HPP
#define THREAD_POOL_PARALLEL_COPY_HPP

#include <cstddef>

void set_up_thread_pool(std::size_t threads_num);

void* parallel_memcpy(void *dst, const void *src, std::size_t size);

#endif
