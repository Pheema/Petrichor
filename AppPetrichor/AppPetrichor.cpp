#include "Core/Petrichor.h"
#include "TestScene/TestScene.h"
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <gflags/gflags.h>
#include <thread>

DEFINE_int32(saveInterval,
             10,
             "rendered file save interval[sec]"); // 0で途中保存なし
DEFINE_int32(numThreads, 1, "number of threads to rendering");
DEFINE_int32(imageWidth, 1280, "output width[px]");
DEFINE_int32(imageHeight, 720, "output image height[px]");
DEFINE_int32(tileSize, 16, "Tile size[px]");
DEFINE_string(imageFileNamePrefix, "Rendered", "output file name prefix");
DEFINE_string(outputPath, "Output/", "relative output path");

void
OnRenderingFinished(const Petrichor::RenderingResult& renderingResult)
{
    std::ofstream file("Output/Result.txt");
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
    using namespace Petrichor;
    using namespace std::chrono_literals;

    constexpr auto kMaxRenderingTime = 123s;
    constexpr auto kSaveInterval = 10s; // 画像を定期的に保存する時間間隔

    Core::Petrichor petrichor;

    Petrichor::Core::Scene scene;
    Petrichor::Core::LoadCornellBoxScene(&scene);
    petrichor.SetRenderCallback(OnRenderingFinished);

#ifdef SAVE_IMAGE_PERIODICALLY
    std::thread saveImage([&] {
        uint32_t idxImg = 0;
        const auto timeBegin = std::chrono::high_resolution_clock::now();
        for (;;)
        {
            std::this_thread::sleep_for(kSaveInterval);
            std::stringstream path;
            path << "Output/" << std::setfill('0') << std::setw(4) << std::right
                 << idxImg++ << ".png";

            Petrichor::Core::Texture2D* targetTexture =
              scene.GetTargetTexture();

            if (targetTexture)
            {
                targetTexture->Save(path.str());
            }
        }
    });
    saveImage.detach();
#endif

#ifdef ENABLE_TIME_LIMITATION
    std::thread elapsedTimeChecker([&] {
        const auto timeBegin = std::chrono::high_resolution_clock::now();

        for (;;)
        {
            std::this_thread::sleep_for(1s);
            const auto now = std::chrono::high_resolution_clock::now();
            const bool isTimeOver =
              std::chrono::duration_cast<std::chrono::seconds>(
                now - timeBegin) > (kMaxRenderingTime - 5s);
            if (isTimeOver)
            {
                petrichor.SaveImage("Output/TimeIsOver.png");
                petrichor.Finalize();
                std::quick_exit(0);
            }
        }
    });
    elapsedTimeChecker.detach();
#endif

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
        targetTexture->Save("Output/Result.png");
    }

    showProgress.join();

    return 0;
}
