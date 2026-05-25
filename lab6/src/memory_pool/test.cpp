#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "../allocator/default_allocator.hpp"
#include "../allocator/pool_allocator.hpp"
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
static inline Node* create_list(unsigned n, Allocator &allocator) {
  Node* list = nullptr;
  for (unsigned i = 0; i < n; i++) {
    list = allocator.construct(list, i);
  }
  return list;
}

template<class Allocator>
static inline void delete_list(Node* list, Allocator &allocator) {
  while (list) {
    Node* node = list;
    list = list->next;
    allocator.deallocate(node);
  }
}

template<class Allocator>
static inline void test(unsigned n, Allocator &allocator) {
  struct rusage start, finish;
  get_usage(start);
  delete_list(create_list(n, allocator), allocator);
  get_usage(finish);

  struct timeval diff;
  timersub(&finish.ru_utime, &start.ru_utime, &diff);
  uint64_t time_used = diff.tv_sec * 1000000 + diff.tv_usec;
  cout << "Time used: " << time_used << " usec\n";

  uint64_t mem_used = (finish.ru_maxrss - start.ru_maxrss) * 1024;
  cout << "Memory used: " << mem_used << " bytes\n";

  auto mem_required = n * sizeof(Node);
  auto overhead = (mem_used - mem_required) * double(100) / mem_used;
  cout << "Overhead: " << std::fixed << std::setw(4) << std::setprecision(1)
       << overhead << "%\n";
}

template<template<class> class Allocator, class... Ts>
void test_allocator(unsigned n, Ts&&... args) {
    Allocator<Node> allocator(std::forward<Ts>(args)...);
    test(n, allocator);
}

int main(const int argc, const char* argv[]) {
  add_overflow_handler();

  constexpr std::size_t LIST_SIZE = 10'000'000;
  constexpr std::size_t ELEMENT_SIZE = sizeof(Node);

  std::cout << "====== Test: default allocator ======" << '\n';
  test_allocator<DefaultAllocator>(LIST_SIZE);

  std::cout << "====== Test: thread unsafe bump pointer allocator ======" << '\n';
  test_allocator<BumpPointerAllocator>(LIST_SIZE, LIST_SIZE * ELEMENT_SIZE, ELEMENT_SIZE);

  return EXIT_SUCCESS;
}
