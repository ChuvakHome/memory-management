//
// #include "thread_local_allocator.hpp"
//
// template<class T>
// BumpPointerThreadLocalAllocator<T>& BumpPointerThreadLocalAllocator<T>::get_instance(std::size_t pool_size, std::size_t block_size) {
//     static thread_local BumpPointerThreadLocalAllocator<T> allocator(pool_size, block_size);
//
//     return allocator;
// }
