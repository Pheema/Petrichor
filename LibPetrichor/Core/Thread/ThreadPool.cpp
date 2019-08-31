#include "ThreadPool.h"

#include "fmt/format.h"
#include <iostream>
#include <unordered_map>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace Petrichor
{
namespace Core
{

ThreadPool::ThreadPool(size_t numThreads)
  : m_numThreads(numThreads)
{
    if (m_numThreads == 0)
    {
        return;
    }

#ifdef _WIN32
    const int numProcessorGroups = GetActiveProcessorGroupCount();

    const auto cumulativeNumProcessors = [numProcessorGroups] {
        std::vector<DWORD> cumulativeNumProcessors_;
        cumulativeNumProcessors_.emplace_back(0);
        for (int groupIndex = 0; groupIndex < numProcessorGroups; groupIndex++)
        {
            const DWORD numProssors = GetActiveProcessorCount(groupIndex);
            fmt::print(
              "Processor group {} has {} cores.\n", groupIndex, numProssors);
            const DWORD prev = cumulativeNumProcessors_.back();
            cumulativeNumProcessors_.emplace_back(prev + numProssors);
        }
        return cumulativeNumProcessors_;
    }();

#endif

    if (numThreads == 0)
    {
        numThreads = std::thread::hardware_concurrency();
    }
    m_threads.reserve(numThreads);

#ifdef _WIN32

    std::vector<DWORD> threadIndexToGroupIndex;
    threadIndexToGroupIndex.resize(numThreads);

    auto iter = cumulativeNumProcessors.cbegin();
    int groupIndex = 0;
    for (DWORD threadIndex = 0; threadIndex < numThreads; threadIndex++)
    {
        if (iter == cumulativeNumProcessors.cend())
        {
            break;
        }

        if (*iter <= threadIndex && threadIndex < *std::next(iter))
        {
            threadIndexToGroupIndex[threadIndex] = groupIndex;
        }
        else
        {
            iter++;
            groupIndex++;
        }
    }

#endif

    for (size_t threadIndex = 0; threadIndex < m_numThreads; threadIndex++)
    {

        int32_t groupIndex = 0;
#ifdef _WIN32
        groupIndex = threadIndexToGroupIndex[threadIndex];
#endif

        m_threads.emplace_back(std::thread([this, threadIndex, groupIndex] {

#ifdef _WIN32
            {
                auto bindThreadToGroup = [](size_t threadIndex,
                                            int groupIndex) {
                    GROUP_AFFINITY groupAffinity{};
                    if (GetNumaNodeProcessorMaskEx(groupIndex, &groupAffinity))
                    {
                        SetThreadGroupAffinity(
                          GetCurrentThread(), &groupAffinity, nullptr);
                    }
                    fmt::print("Thread {} is binded to group {}.\n",
                               threadIndex,
                               groupIndex);
                };
                bindThreadToGroup(threadIndex, groupIndex);
            }
#endif
            for (;;)
            {
                std::function<void(size_t)> task;

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

                task(threadIndex);
            }
        }));
    }
}
ThreadPool::~ThreadPool()
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

void
ThreadPool::Push(std::function<void(size_t)>&& task)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_isTerminated)
        {
            return;
        }

        m_tasks.emplace(std::forward<std::function<void(size_t)>>(task));
    }
    m_cond.notify_one();
}

} // namespace Core
} // namespace Petrichor
