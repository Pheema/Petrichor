#include "Logger.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include <vector>

namespace Petrichor
{
namespace Core
{

void
Logger::AddConsoleOutput()
{
    GetInstance()->sinks().emplace_back(
      std::make_shared<spdlog::sinks::stdout_sink_mt>());
}

void
Logger::AddFileOutput(const std::filesystem::path& logFilePath)
{
    GetInstance()->sinks().emplace_back(
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        logFilePath.string()));
}

} // namespace Core
} // namespace Petrichor
