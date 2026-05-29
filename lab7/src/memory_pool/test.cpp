#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <thread>
#include <vector>
#include <utility>

#include "../allocator/default_allocator.hpp"
#include "../allocator/mutex_allocator.hpp"
#include "../allocator/lock_free_allocator.hpp"
#include "../allocator/thread_local_allocator.hpp"
#include "../overflow_handler/overflow_handler.hpp"

using namespace std;

static void get_usage(struct rusage& usage) {
  if (getrusage(RUSAGE_SELF, &usage)) {
    perror("Cannot get usage");
    exit(EXIT_SUCCESS);
  }
}

struct Node {
  Node* next;
  unsigned node_id;
};

template<class Allocator>
static inline Node* create_list(unsigned n, Allocator *allocator) {
  Node* list = nullptr;
  for (unsigned i = 0; i < n; i++) {
    list = allocator->construct(list, i);
  }
  return list;
}

template<class Allocator>
static inline void delete_list(Node* list, Allocator *allocator) {
  while (list) {
    Node* node = list;
    list = list->next;
    allocator->deallocate(node);
  }
}

template<class Func>
static inline void test(unsigned n, unsigned threads_num, Func&& test_function) {
  struct rusage start, finish;
  get_usage(start);

  std::vector<std::thread> threads;
  threads.reserve(threads_num);

  for (std::size_t i = 0; i < threads_num; ++i) {
    threads.emplace_back([n, &test_function]() {
      std::forward<Func>(test_function)(n);
    });
  }
  for (auto &&t: threads) {
      t.join();
  }
  threads.clear();

  get_usage(finish);

  struct timeval diff;
  timersub(&finish.ru_utime, &start.ru_utime, &diff);
  uint64_t time_used = diff.tv_sec * 1000000 + diff.tv_usec;
  cout << "Time used: " << time_used << " usec\n";

  uint64_t mem_used = (finish.ru_maxrss - start.ru_maxrss) * 1024;
  cout << "Memory used: " << mem_used << " bytes\n";

  auto mem_required = static_cast<double>(threads_num * n * sizeof(Node));
  auto overhead = (mem_used - mem_required) * double(100) / mem_used;
  cout << "Overhead: " << std::fixed << std::setw(4) << std::setprecision(1)
       << overhead << "%\n";
}

template<template<class> class Allocator, class... Ts>
void test_allocator(unsigned n, unsigned threads_num, Ts&&... args) {
    Allocator<Node> allocator(std::forward<Ts>(args)...);
    test(n, threads_num, [mem_allocator = &allocator](unsigned n) {
        delete_list(create_list(n, mem_allocator), mem_allocator);
    });
}

void test_thread_local_allocator(unsigned n, unsigned threads_num, std::size_t pool_size, std::size_t block_size) {
    struct rusage start, finish;
    get_usage(start);

    auto func = [pool_size, block_size, n]() {
        static thread_local BumpPointerThreadLocalAllocator<Node> allocator(
            pool_size, block_size
        );

        create_list(n, &allocator);
        allocator.free_pool();
    };

    std::vector<std::thread> threads;
    threads.reserve(threads_num);

    for (std::size_t i = 0; i < threads_num; ++i) {
        threads.emplace_back(func);
    }

    for (auto &&t: threads) {
        t.join();
    }
    threads.clear();

    get_usage(finish);

    struct timeval diff;
    timersub(&finish.ru_utime, &start.ru_utime, &diff);
    uint64_t time_used = diff.tv_sec * 1000000 + diff.tv_usec;
    cout << "Time used: " << time_used << " usec\n";

    uint64_t mem_used = (finish.ru_maxrss - start.ru_maxrss) * 1024;
    cout << "Memory used: " << mem_used << " bytes\n";

    auto mem_required = static_cast<double>(threads_num * n * sizeof(Node));
    auto overhead = (mem_used - mem_required) * double(100) / mem_used;

    cout << "Overhead: " << std::fixed << std::setw(4) << std::setprecision(1)
         << overhead << "%\n";
}

int main(const int argc, const char* argv[]) {
  add_overflow_handler();

  constexpr std::size_t LIST_SIZE = 10'000'000;
  constexpr std::size_t ELEMENT_SIZE = sizeof(Node);
  constexpr std::size_t THREADS_NUM = 16;

  std::string_view usage = "Usage: ./memory_pool_test {default|mutexed|lock-free|thread-local}\n";

  if (argc < 2) {
      std::cerr << "Too few arguments\n" << usage;

      return EXIT_FAILURE;
  } else if (argc > 2) {
      std::cerr << "Too many arguments\n" << usage;

      return EXIT_FAILURE;
  }

  std::string_view allocator_mode = argv[1];

  if (allocator_mode == "default") {
      std::cout << "====== Test: default allocator ======" << '\n';
      test_allocator<DefaultAllocator>(LIST_SIZE, THREADS_NUM);
  } else if (allocator_mode == "mutexed") {
      std::cout << "====== Test: mutex bump pointer allocator ======" << '\n';
      test_allocator<BumpPointerMutexAllocator>(LIST_SIZE, THREADS_NUM, THREADS_NUM * LIST_SIZE * ELEMENT_SIZE, ELEMENT_SIZE);
  } else if (allocator_mode == "lock-free") {
      std::cout << "====== Test: lock-free bump pointer allocator ======" << '\n';
      test_allocator<BumpPointerLockFreeAllocator>(LIST_SIZE, THREADS_NUM, THREADS_NUM * LIST_SIZE * ELEMENT_SIZE, ELEMENT_SIZE);
  } else if (allocator_mode == "thread-local") {
      std::cout << "====== Test: thread-local bump pointer allocator ======" << '\n';
      test_thread_local_allocator(LIST_SIZE, THREADS_NUM, LIST_SIZE * ELEMENT_SIZE, ELEMENT_SIZE);
  } else {
      std::cerr << "Unknown allocator\n" << usage;
  }

  return EXIT_SUCCESS;
}
