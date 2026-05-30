#ifndef SAFE_QUEUE_SAFE_QUEUE_HPP
#define SAFE_QUEUE_SAFE_QUEUE_HPP

#include <condition_variable>

#include <queue>
#include <mutex>

template <class T>
class SafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;

    std::condition_variable consume_cv_;

public:
    using size_type = typename std::queue<T>::size_type;

    SafeQueue()                          = default;
    SafeQueue(const SafeQueue<T>& other) = default;
    SafeQueue(SafeQueue&& other)         = default;

    ~SafeQueue() {
        wait_for_empty();
    }

    void push(const T& item) {
        std::unique_lock lock(mutex_);
        queue_.push(item);
        cond_.notify_one();
    }

    void push(T&& item) {
        std::unique_lock lock(mutex_);
        queue_.push(std::move(item));
        cond_.notify_one();
    }

    T pop() {
        std::unique_lock lock(mutex_);

        cond_.wait(lock, [this]() { return !queue_.empty(); });

        T item = queue_.front();
        queue_.pop();

        consume_cv_.notify_all();

        return item;
    }

    std::condition_variable& get_cond() {
        return cond_;
    }

    template<class Pred>
    void wait_or(Pred&& predicate) {
        std::unique_lock lock(mutex_);

        cond_.wait(lock, [this, &predicate]() { return !queue_.empty() || predicate(); });
    }

    void wait_for_empty() {
        std::unique_lock lock(mutex_);

        consume_cv_.wait(lock, [this]() { return queue_.empty(); });
    }
};

#endif
