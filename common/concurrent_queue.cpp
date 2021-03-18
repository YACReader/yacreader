#include "concurrent_queue.h"

#include <cassert>
#include <cstddef>
#include <mutex>
#include <utility>

using namespace YACReader;

ConcurrentQueue::ConcurrentQueue(std::size_t threadCount)
{
    threads.reserve(threadCount);
    for (; threadCount != 0; --threadCount)
        threads.emplace_back(&ConcurrentQueue::nextJob, this);
}

ConcurrentQueue::~ConcurrentQueue()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        assert(!bailout);
        bailout = true;
    }
    jobAvailableVar.notify_all();

    for (auto &x : threads)
        x.join();
    assert(jobsLeft == _queue.size() && "Only not yet started jobs are left.");
}

void ConcurrentQueue::enqueue(Job job)
{
    {
        std::lock_guard<std::mutex> lock(jobsLeftMutex);
        ++jobsLeft;
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        _queue.emplace(std::move(job));
    }

    jobAvailableVar.notify_one();
}

std::size_t ConcurrentQueue::cancelPending()
{
    decltype(_queue) oldQueue;
    {
        const std::lock_guard<std::mutex> lock(queueMutex);
        // The mutex locking time is lower with swap() compared to assigning a
        // temporary (which destroys _queue's elements and deallocates memory).
        _queue.swap(oldQueue);
    }

    const auto size = oldQueue.size();
    if (size != 0)
        finalizeJobs(size);
    return size;
}

void ConcurrentQueue::waitAll()
{
    std::unique_lock<std::mutex> lock(jobsLeftMutex);
    _waitVar.wait(lock, [this] { return jobsLeft == 0; });
}

void ConcurrentQueue::nextJob()
{
    while (true) {
        Job job;

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            jobAvailableVar.wait(lock, [this] {
                return _queue.size() > 0 || bailout;
            });

            if (bailout) {
                return;
            }

            job = std::move(_queue.front());
            _queue.pop();
        }

        job();
        finalizeJobs(1);
    }
}

void ConcurrentQueue::finalizeJobs(std::size_t count)
{
    assert(count > 0);

    std::size_t remainingJobs;
    {
        std::lock_guard<std::mutex> lock(jobsLeftMutex);
        assert(jobsLeft >= count);
        jobsLeft -= count;
        remainingJobs = jobsLeft;
    }

    if (remainingJobs == 0)
        _waitVar.notify_all();
}
