
#ifndef THREAD_POOL_THREAD_POOL_HPP
#define THREAD_POOL_THREAD_POOL_HPP

#include <atomic>
#include <cstddef>

#include <cstdio>
#include <stdexcept>
#include <thread>

#include "../safe_queue/safe_queue.hpp"

template<class Task>
class ThreadPool {
public:
    explicit ThreadPool(std::size_t threads_num)
        : threads_num_(threads_num)
        , stopped_(false) {
        thread_workers_.reserve(threads_num);
        for (std::size_t i = 0; i < threads_num; ++i) {
            thread_workers_.emplace_back(&ThreadPool<Task>::thread_routine, this);
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    ~ThreadPool() {
        stop();
    }

    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    std::size_t get_threads_num() const {
        return threads_num_;
    }

    bool is_stopped() const {
        return stopped_;
    }

    void stop() {
        if (stopped_) {
            return;
        }

        stopped_ = true;
        task_queue_.get_cond().notify_all();

        for (auto &&w: thread_workers_) {
            w.join();
        }
    }

    void submit(const Task& task) {
        if (is_stopped()) {
            throw std::runtime_error("submission to the stopped thread-pool");
        }

        task_queue_.push(task);
    }

    void submit(Task&& task) {
        if (is_stopped()) {
            throw std::runtime_error("submission to the stopped thread-pool");
        }

        task_queue_.push(std::move(task));
    }
private:
    std::size_t threads_num_;
    std::atomic_bool stopped_;
    std::vector<std::thread> thread_workers_;
    SafeQueue<Task> task_queue_;

    void thread_routine() {
        while (!stopped_) {
            task_queue_.wait_or([this]() -> bool { return stopped_; });

            if (stopped_) {
                break;
            }

            auto &&f = task_queue_.pop();
            f();
        }
    }
};

#endif
