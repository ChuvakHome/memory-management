
#include <algorithm>
#include <array>
#include <sstream>

#include "simple_test.hpp"

#include "string_ptr.hpp"

TEST(string_ptr, empty_ptr) {
    string_ptr s;
    EXPECT_TRUE(!s.is_unique());

    s = nullptr;
    EXPECT_TRUE(!s.is_unique());
}

TEST(string_ptr, ptr_from_charp) {
    constexpr const char *STRING_DATA = "My string";

    string_ptr s = STRING_DATA;
    EXPECT_TRUE(s.is_unique());
    EXPECT_EQ(s, STRING_DATA);
}

TEST(string_ptr, copy_ctor) {
    constexpr const char *STRING_DATA = "My string";

    string_ptr s = STRING_DATA;
    EXPECT_TRUE(s.is_unique());
    EXPECT_EQ(s, STRING_DATA);

    string_ptr other = s;
    EXPECT_TRUE(!s.is_unique());
    EXPECT_TRUE(!other.is_unique());
    EXPECT_EQ(s.data(), other.data());
    EXPECT_EQ(other, STRING_DATA);
}

TEST(string_ptr, copy_assign) {
    constexpr const char *STRING1_DATA = "My string";
    constexpr const char *STRING2_DATA = "My other string";

    string_ptr s = STRING1_DATA;
    EXPECT_TRUE(s.is_unique());
    EXPECT_EQ(s, STRING1_DATA);

    string_ptr q = STRING2_DATA;
    EXPECT_TRUE(q.is_unique());
    EXPECT_EQ(q, STRING2_DATA);

    q = s;
    EXPECT_TRUE(!s.is_unique());
    EXPECT_TRUE(!q.is_unique());
    EXPECT_EQ(s.data(), q.data());
    EXPECT_EQ(q, STRING1_DATA);
}

TEST(string_ptr, move_ctor) {
    constexpr const char *STRING_DATA = "My string";

    string_ptr s = STRING_DATA;
    EXPECT_TRUE(s.is_unique());
    EXPECT_EQ(s, STRING_DATA);

    string_ptr other = std::move(s);
    EXPECT_TRUE(!s.is_unique());
    EXPECT_TRUE(other.is_unique());
    EXPECT_EQ(s.data(), nullptr);
    EXPECT_EQ(other, STRING_DATA);
}

TEST(string_ptr, move_assign) {
    constexpr const char *STRING1_DATA = "My string";
    constexpr const char *STRING2_DATA = "My other string";

    string_ptr s = STRING1_DATA;
    EXPECT_TRUE(s.is_unique());
    EXPECT_EQ(s, STRING1_DATA);

    string_ptr q = STRING2_DATA;
    EXPECT_TRUE(q.is_unique());
    EXPECT_EQ(q, STRING2_DATA);

    q = std::move(s);
    EXPECT_TRUE(!s.is_unique());
    EXPECT_TRUE(q.is_unique());
    EXPECT_EQ(s.data(), nullptr);
    EXPECT_EQ(q, STRING1_DATA);
}

TEST(string_ptr, string_print) {
    constexpr const char *STRING_DATA = "My string";
    string_ptr s = STRING_DATA;
    EXPECT_TRUE(s.is_unique());
    EXPECT_EQ(s, STRING_DATA);

    std::ostringstream oss;
    oss << s;
    EXPECT_TRUE(std::strcmp(oss.str().data(), "My string[unique = true]") == 0);
}

namespace {
    template<std::size_t N>
    void bubble_sort(std::array<string_ptr, N> &arr) {
        for (std::size_t i = 0; i < arr.size(); ++i) {
            for (std::size_t j = 0; j + 1 < arr.size() - i; ++j) {
                if (arr[j] > arr[j + 1]) {
                    std::swap(arr[j], arr[j + 1]);
                }
            }
        }
    }

    template<std::size_t N>
    void save_unique_mark_bits(const std::array<string_ptr, N> &arr, std::array<bool, N> &unique_mark_bits) {
        std::transform(arr.begin(), arr.end(), unique_mark_bits.begin(), [](const string_ptr &s){
            return s.is_unique();
        });
    }
}

TEST(string_ptr, bubble_sort) {
    constexpr std::size_t ARRAY_SIZE = 10;

    std::array<string_ptr, ARRAY_SIZE> arr{
        "Lorem ipsum dolor sit amet",
        "Excepteur sint occaecat cupidatat non proident",
        "quis aliquip minim aliquip exercitation",
        "Culpa laborum culpa enim dolore nostrud velit",
        "ullamco dolor exercitation mollit ut",

        "lorem esse ullamco esse do occaecat occaecat fugiat",
        "Proident incididunt do adipiscing consequat",
        "Commodo pariatur lorem dolor et incididunt",
        "Non cillum aute veniam elit veniam cupidatat",
        "reprehenderit id ad nulla cillum sit cillum proident consequat"
    };
    std::array<bool, arr.size()> old_unique_flags;
    save_unique_mark_bits(arr, old_unique_flags);

    bubble_sort(arr);

    std::array<bool, arr.size()> new_unique_flags;
    save_unique_mark_bits(arr, new_unique_flags);

    EXPECT_EQ(old_unique_flags, new_unique_flags);

    for (std::size_t i = 0; i < arr.size() - 1; ++i) {
        EXPECT_LE(arr[i], arr[i + 1]);
    }
}

TESTING_MAIN()
