#ifndef SEMAPHORE_H
#define SEMAPHORE_H

// Mutex is used to protect the internal counter.
#include <mutex>
// Condition variable is used to block/wake waiters when counter changes.
#include <condition_variable>

// A simple counting semaphore implementation for limiting concurrent access to a resource.
class Semaphore {
private:
    // Guards access to `count_` and coordinates waiting.
    std::mutex mutex_;
    // Used to wait until `count_` becomes positive, and to wake one waiter on release.
    std::condition_variable cv_;
    // Current number of available "permits" (resource slots).
    int count_;

public:
    // Construct the semaphore with an initial number of available permits.
    explicit Semaphore(int initialCount) : count_(initialCount) {}

    void acquire() {
        // Take the mutex so we can safely check and modify the counter.
        std::unique_lock<std::mutex> lock(mutex_);
        // Block until a permit is available; the predicate re-checks under the lock after wakeups.
        cv_.wait(lock, [this]() { return count_ > 0; });
        // Consume one permit now that we're allowed to proceed.
        --count_;
    }

    void release() {
        // Lock scope: update the counter while holding the mutex.
        {
            // Use a lock_guard since we only need a simple scoped lock here.
            std::lock_guard<std::mutex> lock(mutex_);
            // Return one permit to the semaphore.
            ++count_;
        }
        // Wake a single waiting thread (if any) so it can attempt to acquire a permit.
        cv_.notify_one();
    }
};

#endif
