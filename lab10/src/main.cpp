
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>

#include "thread_pool/parallel_copy.hpp"

int main() {
    constexpr std::size_t SIZE = 256 * 1024 * 1024;

    std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<std::uint8_t> dist(0, 255);

    for (std::size_t threads_num = 0; threads_num <= 0; ++threads_num) {
        std::cout << "====== Test with " << threads_num << " threads ====== \n";

        set_up_thread_pool(threads_num);

        std::vector<std::byte> src(SIZE);
        std::vector<std::byte> dst(SIZE);

        std::generate(src.begin(), src.end(), [&dist, &rng](){
            return static_cast<std::byte>(dist(rng));
        });

        auto start_time = std::chrono::high_resolution_clock::now();

        parallel_memcpy(dst.data(), src.data(), SIZE);

        auto end_time = std::chrono::high_resolution_clock::now();

        assert(src == dst);

        auto elapsed_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count();
        std::cout << std::setprecision(2) << "Elapsed: " << elapsed_ms << " ms\n";
    }

    return 0;
}
