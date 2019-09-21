#pragma once

#include "spdlog/spdlog.h"
#include <chrono>
#include <filesystem>
#include <memory>
#include <string_view>

namespace Petrichor
{
namespace Core
{

class Logger
{
public:
    Logger() = default;

    static void
    AddConsoleOutput();

    static void
    AddFileOutput(const std::filesystem::path& logFilePath);

    template<typename T>
    static void
    Info(const T& log)
    {
        GetInstance()->info(log);
    }

    template<typename... Args>
    static void
    Info(std::string_view fmt, const Args&... args)
    {
        GetInstance()->info(fmt, args...);
    }

    template<typename T>
    static void
    Error(const T& log)
    {
        GetInstance()->error(log);
    }

    template<typename... Args>
    static void
    Error(std::string_view fmt, const Args&... args)
    {
        GetInstance()->error(fmt, args...);
    }

private:
private:
    static std::shared_ptr<spdlog::logger>&
    GetInstance()
    {
        static auto logger = std::make_shared<spdlog::logger>("logger");
        return logger;
    }
};

class ScopeLogger
{
public:
    ScopeLogger(const std::string& scopeName)
      : m_scopeName(scopeName)
    {
        m_clock = std::chrono::high_resolution_clock::now();
        Logger::Info("[BEGIN SCOPE] {}", m_scopeName);
    }
    ~ScopeLogger()
    {
        const auto eplasedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - m_clock);
        Logger::Info("[END SCOPE] {} - elpasedtime: {}[ms]",
                     m_scopeName,
                     eplasedTime.count());
    }

private:
    std::chrono::high_resolution_clock::time_point m_clock;
    std::string m_scopeName;
};

} // namespace Core
} // namespace Petrichor

#define PETRICHOR_CONCAT_IMPL(x, y) x##y
#define PETRICHOR_MACRO_CONCAT(x, y) PETRICHOR_CONCAT_IMPL(x, y)
#define SCOPE_LOGGER(scopeName)                                                \
    ::Petrichor::Core::ScopeLogger PETRICHOR_MACRO_CONCAT(                     \
      scopeLogger, __COUNTER__)(scopeName)
