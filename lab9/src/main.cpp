#include <cstdint>

#include "safe_read.hpp"

#include "simple_test.hpp"

namespace {
    std::string stringify_optional(const std::optional<std::uint8_t> &opt) {
        if (opt.has_value()) {
            return std::to_string(static_cast<unsigned>(*opt));
        } else {
            return "[nullopt]";
        }
    }
}

TEST(safe_read_u8, read_ptr_from_stack) {
    std::uint8_t x = 42;
    std::optional<std::uint8_t> opt = safe_read_uint8(&x);

    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(*opt, x);
}

TEST(safe_read_u8, read_ptr_from_heap) {
    std::uint8_t *x = new std::uint8_t{42};
    std::optional<std::uint8_t> opt = safe_read_uint8(x);

    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(*opt, *x);

    delete x;
}

TEST(safe_read_u8, read_nullptr) {
    std::optional<std::uint8_t> opt = safe_read_uint8(nullptr);

    EXPECT_TRUE(!opt.has_value());
}

TESTING_MAIN()
