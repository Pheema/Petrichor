#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Petrichor
{
namespace Core
{

class ThreadPool
{
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    void
    Push(std::function<void(size_t)>&& task);

private:
    std::vector<std::thread> m_threads;
    std::queue<std::function<void(size_t)>> m_tasks;
    const size_t m_numThreads;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_isTerminated = false;
};

} // namespace Core
} // namespace Petrichor
