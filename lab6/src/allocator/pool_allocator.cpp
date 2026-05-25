//
// #include "pool_allocator.hpp"
//
// #include <algorithm>
// #include <cassert>
// #include <cstdio>
// #include <unistd.h>
//
// #include <sys/mman.h>
//
// namespace {
//     const std::size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
//
//     inline std::size_t align_size(std::size_t sz, std::size_t alignment) {
//         assert((alignment & (alignment - 1)) != 0); // align should be a power of 2
//
//         return sz + std::clamp<std::size_t>(sz & (alignment - 1), 0, 1);
//     }
// }
//
// static void sigsegv_handler(int) {
//
// }
//
// template<class T>
// BumpPointerAllocator<T>::BumpPointerAllocator(std::size_t pool_size, std::size_t block_size) {
//     std::size_t actual_pool_size = align_size(pool_size, PAGE_SIZE);
//     std::size_t aligned_block_size = align_size(block_size, PAGE_SIZE);
//
//     pool_size_ = actual_pool_size + aligned_block_size,
//     first_free_ = static_cast<std::byte *>(mmap(
//         nullptr,
//         pool_size_,
//         PROT_READ | PROT_WRITE,
//         MAP_ANON | MAP_PRIVATE,
//         -1,
//         0
//     ));
//
//     if (first_free_ == MAP_FAILED) {
//         perror("Initial memory allocation failed");
//     }
//
//     first_free_ += pool_size_;
//     mprotect(first_free_, pool_size_, PROT_NONE);
// }
