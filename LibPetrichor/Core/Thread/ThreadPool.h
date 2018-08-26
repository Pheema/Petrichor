#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Petrichor
{
namespace Core
{

template<class T>
class ThreadPool
{
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    void
    Run(std::function<T()>&& task);

private:
    std::vector<std::thread> m_threads;
    std::queue<std::function<T()>> m_tasks;
    const size_t m_numThreads;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_isTerminated = false;
};

template<class T>
ThreadPool<T>::ThreadPool(size_t numThreads)
  : m_numThreads(numThreads)
{
    if (m_numThreads == 0)
    {
        return;
    }

    m_threads.reserve(numThreads);
    for (size_t threadIndex = 0; threadIndex < m_numThreads; threadIndex++)
    {
        m_threads.emplace_back(std::thread([this] {
            for (;;)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cond.wait(lock, [this] {
                        return !m_tasks.empty() || m_isTerminated;
                    });

                    if (m_tasks.empty() && m_isTerminated)
                    {
                        return;
                    }

                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                task();
            }
        }));
    }
}

template<class T>
ThreadPool<T>::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_isTerminated = true;
    }
    m_cond.notify_all();

    for (auto& thread : m_threads)
    {
        thread.join();
    }
}

template<class T>
void
ThreadPool<T>::Run(std::function<T()>&& task)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_isTerminated)
        {
            return;
        }

        m_tasks.emplace(std::forward<std::function<T()>>(task));
    }
    m_cond.notify_one();
}

} // namespace Core
} // namespace Petrichor
