#include "ThreadPool.h"

#include "Core/Logger.h"
#include <iostream>

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
{
    if (numThreads == 0)
    {
#ifdef _WIN32
        numThreads = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
#else
        numThreads = std::thread::hardware_concurrency();
#endif
    }

    m_numThreads = numThreads;

#ifdef _WIN32
    const WORD numProcessorGroups = GetActiveProcessorGroupCount();
    const auto cumulativeNumProcessors = [numProcessorGroups] {
        std::vector<DWORD> cumulativeNumProcessors_;
        cumulativeNumProcessors_.emplace_back(0);
        for (int groupIndex = 0; groupIndex < numProcessorGroups; groupIndex++)
        {
            const DWORD numProssors = GetActiveProcessorCount(groupIndex);
            Logger::Info(
              "Processor group {} has {} cores.", groupIndex, numProssors);
            const DWORD prev = cumulativeNumProcessors_.back();
            cumulativeNumProcessors_.emplace_back(prev + numProssors);
        }
        return cumulativeNumProcessors_;
    }();

#endif

    m_threads.reserve(m_numThreads);

#ifdef _WIN32
    std::vector<DWORD> threadIndexToGroupIndex;
    threadIndexToGroupIndex.resize(numThreads);

    DWORD groupIndex = 0;
    for (DWORD threadIndex = 0; threadIndex < numThreads; threadIndex++)
    {
        if (cumulativeNumProcessors.size() <= groupIndex + 1)
        {
            break;
        }

        const DWORD lowerThreadIndex = cumulativeNumProcessors[groupIndex];
        const DWORD upperThreadIndex = cumulativeNumProcessors[groupIndex + 1];

        if (threadIndex <= threadIndex && threadIndex < upperThreadIndex)
        {
            threadIndexToGroupIndex[threadIndex] = groupIndex;
        }
        else
        {
            groupIndex++;
            threadIndexToGroupIndex[threadIndex] = groupIndex;
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
                    Logger::Info("Thread {} is binded to group {}.",
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
