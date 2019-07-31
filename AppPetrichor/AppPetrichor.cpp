#include "Core/Geometry/Sphere.h"
#include "Core/Petrichor.h"
#include "TestScene/TestScene.h"
#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <fstream>
#include <gflags/gflags.h>
#include <thread>

DEFINE_string(outputDir, "./Output", "relative output path");
DEFINE_uint32(timeLimit, 0, "Rendering time limit");
DEFINE_string(renderSettingPath, "settings.json", "Render setting file path.");
DEFINE_string(assetsPath, "assets.json", "Render setting file path.");

std::string
GetCurrentTimeString()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};

#if defined(_WIN32)
    localtime_s(&tm, &time);
#elif defined(__linux__)
    localtime_r(&time, &tm);
#else
    tm = *std::localtime(&time);
#endif

    return fmt::format("{:04d}{:02d}{:02d}_{:02d}{:02d}{:02d}",
                       tm.tm_year + 1900,
                       tm.tm_mon + 1,
                       tm.tm_mday,
                       tm.tm_hour,
                       tm.tm_min,
                       tm.tm_sec);
}

void
OnRenderingFinished(const Petrichor::RenderingResult& renderingResult)
{
    std::filesystem::path outputDir(FLAGS_outputDir);
    const std::string filename = GetCurrentTimeString() + ".txt";

    std::ofstream file(outputDir / filename);
    if (file.fail())
    {
        return;
    }

    file << fmt::format("Total time: {:.2f} [s]\n", renderingResult.totalSec);
    file.close();
}

int
main(int argc, char** argv)
{
    using namespace std::chrono_literals;
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Petrichor::Core::Scene scene;
    scene.LoadRenderSetting(FLAGS_renderSettingPath);
    scene.LoadAssets(FLAGS_assetsPath);

    // #TODO: ライトの設定等が仮で直書きになっているのでjson側へ逃がす
    {
        using namespace Petrichor;
        using namespace Petrichor::Core;

        // レンダリング先を指定
        auto targetTex = new Texture2D(scene.GetRenderSetting().outputWidth,
                                       scene.GetRenderSetting().outputHeight);
        scene.SetTargetTexture(targetTex);
    }

    Petrichor::Core::Petrichor petrichor;
    petrichor.SetRenderCallback(OnRenderingFinished);

    std::filesystem::path outputDir(FLAGS_outputDir);
    if (!std::filesystem::is_directory(outputDir))
    {
        fmt::print("Invalid output directory.\n");
        return 1;
    }

    // 時間制限あればセット
    if (FLAGS_timeLimit > 0)
    {
        const auto timeLimit = std::chrono::seconds(FLAGS_timeLimit);

        std::thread elapsedTimeChecker([timeLimit, &scene, &outputDir] {
            const auto timeBegin = std::chrono::high_resolution_clock::now();
            for (;;)
            {
                std::this_thread::sleep_for(0.1s);
                const auto now = std::chrono::high_resolution_clock::now();
                const bool isTimeOver =
                  std::chrono::duration_cast<std::chrono::seconds>(
                    now - timeBegin) > (timeLimit - 1s);

                if (isTimeOver)
                {
                    Petrichor::Core::Texture2D* targetTexture =
                      scene.GetTargetTexture();
                    if (targetTexture)
                    {
                        targetTexture->Save(outputDir / "TimeOver.png");
                    }
                    fmt::print("File saved: ({})\n",
                               (outputDir / "TimeOver.png").string());
                    std::quick_exit(0);
                }
            }
        });
        elapsedTimeChecker.detach();
    }

    fmt::print("Hardware Concurrency: {}\n",
               std::thread::hardware_concurrency());
    fmt::print("Number of used threads: {}\n",
               scene.GetRenderSetting().numThreads);

    const auto progressBar = [](float ratio) {
        std::string progressBar;
        constexpr int kMaxNumChars = 50;
        const auto numBlockChars = static_cast<int>(kMaxNumChars * ratio);

        progressBar += "[";
        for (int charPos = 0; charPos < numBlockChars; charPos++)
        {
            progressBar += "*";
        }
        for (int charPos = numBlockChars; charPos < kMaxNumChars; charPos++)
        {
            progressBar += " ";
        }
        progressBar += "]";

        return progressBar;
    };

    std::thread showProgress([&] {
        for (;;)
        {
            std::this_thread::sleep_for(0.1s);

            if (petrichor.GetNumTiles() == 0)
            {
                continue;
            }

            const float ratio =
              static_cast<float>(petrichor.GetNumRenderedTiles()) /
              petrichor.GetNumTiles();
            fmt::print("[Progress]: {:.2f}% ({} / {}) {}\r",
                       100.0f * ratio,
                       petrichor.GetNumRenderedTiles(),
                       petrichor.GetNumTiles(),
                       progressBar(ratio));
            std::cout << std::flush;

            if (petrichor.GetNumRenderedTiles() >= petrichor.GetNumTiles())
            {
                return;
            }
        }
    });

    petrichor.Render(scene);

    Petrichor::Core::Texture2D* targetTexture = scene.GetTargetTexture();
    if (targetTexture)
    {
        const std::string filename = GetCurrentTimeString() + ".png";
        targetTexture->Save(outputDir / filename);
    }

    showProgress.join();

    return 0;
}
