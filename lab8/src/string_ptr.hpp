
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
    static constexpr std::uintptr_t POINTER_MASK = ~static_cast<std::uintptr_t>(1);

    inline const char* copy_string_aligned(const char *src, std::align_val_t alignment = STRING_PTR_ALIGNMENT) {
        char* dst = nullptr;

        if (src != nullptr) {
            const std::size_t sz = std::strlen(src);
            dst = new(alignment) char[sz + 1]();

            std::strncpy(dst, src, sz);
        }

        return dst;
    }

    inline const void *get_raw_ptr(const void *src) {
        return src == nullptr
                ? nullptr
                : reinterpret_cast<const void *>(
                    (reinterpret_cast<std::uintptr_t>(src) & POINTER_MASK)
                );
    }

    const void *get_raw_ptr(void *src) {
        return src == nullptr
                ? nullptr
                : reinterpret_cast<void *>(
                    (reinterpret_cast<std::uintptr_t>(src) & POINTER_MASK)
                );
    }

    const void* get_ptr_with_mark_bit(const void *src, bool mark_bit) {
        return src == nullptr
                ? nullptr
                : reinterpret_cast<const void *>(
                    reinterpret_cast<std::uintptr_t>(get_raw_ptr(src)) | (mark_bit ? 1 : 0)
                );
    }
}

struct string_ptr;
inline std::ostream & operator<<(std::ostream &os, const string_ptr &s);

struct string_ptr {
private:
    mutable const char *ptr_;
public:
    string_ptr(const char *src = nullptr)
        : ptr_(copy_string_aligned(src, STRING_PTR_ALIGNMENT)) {
        DEBUG_LOG("Allocate string_ptr from [" << static_cast<const void *>(src) << "]\n");
        ptr_ = static_cast<const char *>(get_ptr_with_mark_bit(ptr_, true));
    }

    string_ptr(const string_ptr &other)
        : ptr_(static_cast<const char *>(get_ptr_with_mark_bit(other.ptr_, false))) {
        DEBUG_LOG("Copy string_ptr from (" << other << ")\n");
        other.ptr_ = ptr_;
    }

    string_ptr(string_ptr&& other) noexcept
        : ptr_(other.ptr_) {
        DEBUG_LOG("Copy string_ptr from rvalue-ref(" << other << ")\n");
        other.ptr_ = static_cast<const char *>(get_ptr_with_mark_bit(nullptr, false));
    }

    ~string_ptr() noexcept {
        DEBUG_LOG("Deleting string_ptr (" << *this << ")\n");
        free_string_if_needed();
    }

    string_ptr& operator=(const char *src) {
        free_string_if_needed();

        DEBUG_LOG("Assign string_ptr from [" << static_cast<const void *>(src) << "]\n");

        if (src != nullptr) {
            ptr_ = copy_string_aligned(src);
            ptr_ = static_cast<const char *>(get_ptr_with_mark_bit(ptr_, true));
        }

        return *this;
    }

    string_ptr& operator=(const string_ptr &other) {
        DEBUG_LOG("Copy-assign string_ptr from (" << other << ")\n");

        if (this != &other) {
            free_string_if_needed();

            ptr_ = static_cast<const char *>(get_ptr_with_mark_bit(other.ptr_, false));
            other.ptr_ = ptr_;
        }

        return *this;
    }

    string_ptr& operator=(string_ptr&& other) noexcept {
        DEBUG_LOG("Move-assign string_ptr from (" << other << ")\n");

        if (this != &other) {
            free_string_if_needed();

            ptr_ = other.ptr_;
            other.ptr_ = static_cast<const char *>(get_ptr_with_mark_bit(nullptr, false));
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
        return (reinterpret_cast<std::uintptr_t>(ptr_) & 1) != 0;
    }

    bool is_non_unique() const {
        return (reinterpret_cast<std::uintptr_t>(ptr_) & 1) == 0;
    }

    const char *data() const {
        return static_cast<const char *>(get_raw_ptr(ptr_));
    }
private:
    void free_string_if_needed() {
        DEBUG_LOG("Deleting string requested\n");

        if (is_unique()) {
            DEBUG_LOG("Deallocating unique string [" << get_raw_ptr(static_cast<const void *>(ptr_)) << "]\n");
            delete[] data();
        }

        ptr_ = nullptr;
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
