#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <queue>

namespace YACReader {
class ConcurrentQueue
{
public:
    explicit ConcurrentQueue(int threadCount)
        : jobsLeft(0),
          bailout(false)
    {
        threads = std::vector<std::thread>(threadCount);
        for (int index = 0; index < threadCount; ++index) {
            threads[index] = std::thread([this] {
                this->nextJob();
            });
        }
    }

    ~ConcurrentQueue()
    {
        joinAll();
    }

    void enqueue(std::function<void(void)> job)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            _queue.emplace(job);
        }

        {
            std::lock_guard<std::mutex> lock(jobsLeftMutex);
            ++jobsLeft;
        }

        jobAvailableVar.notify_one();
    }

    void cancellPending()
    {
        std::unique_lock<std::mutex> lockQueue(queueMutex);
        std::unique_lock<std::mutex> lockJobsLeft(jobsLeftMutex);
        _queue = std::queue<std::function<void(void)>>();
        jobsLeft = 0;
    }

    void waitAll()
    {
        std::unique_lock<std::mutex> lock(jobsLeftMutex);
        if (jobsLeft > 0) {
            _waitVar.wait(lock, [this] {
                return jobsLeft == 0;
            });
        }
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void(void)>> _queue;
    int jobsLeft;
    bool bailout;
    std::condition_variable jobAvailableVar;
    std::condition_variable _waitVar;
    std::mutex jobsLeftMutex;
    std::mutex queueMutex;

    void nextJob()
    {
        while (true) {
            std::function<void(void)> job;

            {
                std::unique_lock<std::mutex> lock(queueMutex);

                if (bailout) {
                    return;
                }

                jobAvailableVar.wait(lock, [this] {
                    return _queue.size() > 0 || bailout;
                });

                if (bailout) {
                    return;
                }

                job = _queue.front();
                _queue.pop();
            }

            job();

            {
                std::lock_guard<std::mutex> lock(jobsLeftMutex);
                --jobsLeft;
            }

            _waitVar.notify_one();
        }
    }

    void joinAll()
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);

            if (bailout) {
                return;
            }

            bailout = true;
        }

        jobAvailableVar.notify_all();

        for (auto &x : threads) {
            if (x.joinable()) {
                x.join();
            }
        }
    }
};

}

#endif // CONCURRENT_QUEUE_H
