
#ifndef DEFAULT_ALLOCATOR_HPP
#define DEFAULT_ALLOCATOR_HPP

#include <new>

template<class T>
class DefaultAllocator {
public:
    DefaultAllocator() = default;
    DefaultAllocator(const DefaultAllocator &other) = delete;

    ~DefaultAllocator() = default;

    DefaultAllocator& operator=(const DefaultAllocator &other) = delete;

    T* allocate() {
        return static_cast<T *>(new(std::align_val_t{alignof(T)}) std::byte[sizeof(T)]());
    }

    template<class... Ts>
    T* construct(Ts&&... args) {
        return new T{std::forward<Ts>(args)...};
    }

    void deallocate(T *ptr) {
        delete ptr;
    }
};

#endif
