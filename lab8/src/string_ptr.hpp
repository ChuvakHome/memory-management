
#ifndef STRING_PTR_HPP
#define STRING_PTR_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <ostream>
#include <string_view>

#ifdef DEBUG
#include <iostream>
#define DEBUG_LOG(MSG) std::cerr << MSG
#else
#define DEBUG_LOG(MSG)
#endif

namespace {
    static constexpr std::align_val_t STRING_PTR_ALIGNMENT = std::align_val_t{2};

    static constexpr std::uintptr_t UNIQUE_BIT_MASK = 1;
    static constexpr std::uintptr_t POINTER_MASK = ~UNIQUE_BIT_MASK;

    static const std::uintptr_t NULL_UINTPTR = reinterpret_cast<std::uintptr_t>(nullptr);

    inline const char *get_raw_ptr(std::uintptr_t p) {
        return reinterpret_cast<const char *>(p & POINTER_MASK);
    }

    inline std::uintptr_t clear_unique_bit(std::uintptr_t p) {
        return p & POINTER_MASK;
    }

    constexpr inline std::uintptr_t set_unique_bit(std::uintptr_t p) {
        return ((p & POINTER_MASK) | UNIQUE_BIT_MASK);
    }

    inline std::uintptr_t copy_string_aligned(const char *src, std::align_val_t alignment = STRING_PTR_ALIGNMENT) {
        if (src != nullptr) {
            const std::size_t sz = std::strlen(src);
            char *dst = new(alignment) char[sz + 1]();

            std::memcpy(dst, src, sz);

            return set_unique_bit(reinterpret_cast<std::uintptr_t>(dst));
        }

        return NULL_UINTPTR;
    }
}

struct string_ptr;
inline std::ostream & operator<<(std::ostream &os, const string_ptr &s);

struct string_ptr {
private:
    mutable std::uintptr_t ptr_;
public:
    string_ptr(const char *src = nullptr)
        : ptr_(copy_string_aligned(src, STRING_PTR_ALIGNMENT)) {
        DEBUG_LOG("Allocate string_ptr from [" << static_cast<const void *>(src) << "]\n");
    }

    string_ptr(const string_ptr &other)
        : ptr_(clear_unique_bit(other.ptr_)) {
        DEBUG_LOG("Copy string_ptr from (" << other << ")\n");
        other.ptr_ = ptr_;
    }

    string_ptr(string_ptr&& other) noexcept
        : ptr_(other.ptr_) {
        DEBUG_LOG("Copy string_ptr from rvalue-ref(" << other << ")\n");
        other.ptr_ = NULL_UINTPTR;
    }

    ~string_ptr() noexcept {
        DEBUG_LOG("Deleting string_ptr (" << *this << ")\n");
        free_string_if_needed();
    }

    string_ptr& operator=(const char *src) {
        free_string_if_needed();

        DEBUG_LOG("Assign string_ptr from [" << static_cast<const void *>(src) << "]\n");
        ptr_ = copy_string_aligned(src);

        return *this;
    }

    string_ptr& operator=(const string_ptr &other) {
        DEBUG_LOG("Copy-assign string_ptr from (" << other << ")\n");

        if (this != &other) {
            free_string_if_needed();

            ptr_ = clear_unique_bit(other.ptr_);
            other.ptr_ = ptr_;
        }

        return *this;
    }

    string_ptr& operator=(string_ptr&& other) noexcept {
        DEBUG_LOG("Move-assign string_ptr from (" << other << ")\n");

        if (this != &other) {
            free_string_if_needed();

            ptr_ = other.ptr_;
            other.ptr_ = NULL_UINTPTR;
        }

        return *this;
    }

    std::strong_ordering operator<=>(const string_ptr &other) const noexcept {
        return std::string_view{data()} <=> std::string_view{other.data()};
    }

    std::strong_ordering operator<=>(std::string_view sv) const {
        return std::string_view{data()} <=> sv;
    }

    std::strong_ordering operator<=>(const char *s) const {
        return std::string_view{data()} <=> std::string_view{s};
    }

    bool is_unique() const {
        return (ptr_ & UNIQUE_BIT_MASK) != 0;
    }

    bool is_non_unique() const {
        return (ptr_ & UNIQUE_BIT_MASK) == 0;
    }

    const char *data() const {
        return get_raw_ptr(ptr_);
    }
private:
    void free_string_if_needed() {
        DEBUG_LOG("String deletion requested\n");

        if (is_unique()) {
            DEBUG_LOG("Deallocating unique string [" << get_raw_ptr(static_cast<const void *>(ptr_)) << "]\n");
            delete[] data();
        }

        ptr_ = NULL_UINTPTR;
    }
};

inline std::ostream & operator<<(std::ostream &os, const string_ptr &s) {
    if (s.data() == nullptr) {
        os << static_cast<const void *>(s.data()) << "[unique = false]";
    } else {
        os << s.data() << "[unique = " << (s.is_unique() ? "true" : "false") << "]";
    }

    return os;
}

inline bool operator==(const string_ptr &s1, const string_ptr &s2) noexcept {
    return std::string_view{s1.data()} == std::string_view{s2.data()};
}

inline bool operator==(const string_ptr &s, std::string_view sv) noexcept {
    return std::string_view{s.data()} == sv;
}

inline bool operator==(const string_ptr &s1, const char *s2) noexcept {
    return std::string_view{s1.data()} == std::string_view{s2};
}

#endif
