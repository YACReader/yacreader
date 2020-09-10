#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include "release_acquire_atomic.h"

#include <cassert>
#include <utility>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

//! Usage:
//!  1. call performTask();
//!  2. wait until busy() returns false;
//!  3. (optionally) call extractResult();
//!  4. return to step 1 (assign another task).
//!  You may invoke busy() and the destructor at any moment.
template<typename Result>
class WorkerThread
{
public:
    WorkerThread() = default;
    ~WorkerThread();

    using Task = std::function<Result()>;
    void performTask(Task newTask);

    //! @return true if the last assigned task is not done yet.
    bool busy() const { return working; }
    Result extractResult() { return std::move(result); }

private:
    void run();

    Task task;
    Result result;

    ReleaseAcquireAtomic<bool> working { false };
    bool abort { false };
    std::mutex mutex;
    std::condition_variable condition;
    std::thread thread;
};

template<typename Result>
WorkerThread<Result>::~WorkerThread()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        abort = true;
    }
    condition.notify_one();
    if (thread.joinable()) {
        thread.join();
    }
}

template<typename Result>
void WorkerThread<Result>::performTask(Task newTask)
{
    assert(!working && "Don't interrupt my work!");
    assert(newTask && "The task may not be empty.");
    task = std::move(newTask);

    if (thread.joinable()) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            working = true;
        }
        condition.notify_one();
    } else {
        working = true;
        thread = std::thread(&WorkerThread::run, this);
    }
}

template<typename Result>
void WorkerThread<Result>::run()
{
    while (true) {
        result = task();
        working = false;

        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this] { return working || abort; });
        if (abort)
            break;
    }
}

#endif // WORKER_THREAD_H
