#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <queue>
#include <vector>

namespace YACReader {
//! All functions in this class are thread-safe in the Qt documentation sense.
class ConcurrentQueue
{
public:
    //! @brief Creates and starts executing @p threadCount worker threads.
    //! @note ConcurrentQueue is unable to execute jobs if @p threadCount == 0.
    explicit ConcurrentQueue(std::size_t threadCount);

    //! Cancels all jobs that have not been picked up by worker threads yet,
    //! waits for all worker threads to complete their jobs and joins them.
    ~ConcurrentQueue();

    using Job = std::function<void()>;

    //! @brief Adds @p job to the queue.
    //! @note A worker thread may start executing @p job immediately if it is idle.
    //! Worker threads start executing jobs in the same order as they are enqueued.
    void enqueue(Job job);

    //! @brief Cancels all jobs that have not been picked up by worker threads yet.
    //! @return The number of jobs that were canceled.
    std::size_t cancelPending();

    //! @brief Blocks the current thread until all enqueued jobs are completed.
    void waitAll();

private:
    std::vector<std::thread> threads;
    std::queue<Job> _queue;
    std::size_t jobsLeft = 0; //!< @invariant jobsLeft >= _queue.size()
    bool bailout = false;
    std::condition_variable jobAvailableVar;
    std::condition_variable _waitVar;
    std::mutex jobsLeftMutex;
    std::mutex queueMutex;

    void nextJob();
    void finalizeJobs(std::size_t count);
    void joinAll();
};

}

#endif // CONCURRENT_QUEUE_H
