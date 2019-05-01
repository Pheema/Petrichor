#include "Core/Petrichor.h"
#include "TestScene/TestScene.h"
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <gflags/gflags.h>
#include <iomanip>
#include <sstream>
#include <thread>

// DEFINE_int32(saveInterval,
//             10,
//             "rendered file save interval[sec]"); // 0で途中保存なし
// DEFINE_int32(numThreads, 1, "number of threads to rendering");
// DEFINE_int32(imageWidth, 1280, "output width[px]");
// DEFINE_int32(imageHeight, 720, "output image height[px]");
// DEFINE_int32(tileSize, 16, "Tile size[px]");
// DEFINE_string(imageFileNamePrefix, 16, "output file name prefix");
// DEFINE_string(outputPath, "Output/", "relative output path");

void
OnRenderingFinished(const Petrichor::RenderingResult& renderingResult)
{
    std::ofstream file("Output/Result.txt");
    if (file.fail())
    {
        return;
    }

    file << std::fixed;
    file << "Total time: " << std::setprecision(2) << renderingResult.totalSec
         << "[s]" << std::endl;
    file.close();
}

int
main(int argc, char argv[])
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

    std::thread showProgress([&] {
        for (;;)
        {
            std::this_thread::sleep_for(0.1s);
            std::cout << "[Tile]: " << petrichor.GetNumRenderedTiles() << " / "
                      << petrichor.GetNumTiles() << "\r";
        }
    });
    showProgress.detach();

    std::cout << "Hardware Concurrency: " << std::thread::hardware_concurrency()
              << std::endl;
    std::cout << "Number of used threads: "
              << scene.GetRenderSetting().numThreads << std::endl;

    petrichor.Render(scene);

    Petrichor::Core::Texture2D* targetTexture = scene.GetTargetTexture();
    if (targetTexture)
    {
        targetTexture->Save("Output/Result.png");
    }
    return 0;
}
