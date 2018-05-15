#include "Core/Petrichor.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <cstdlib>

// #define ENABLE_TIME_LIMITATION
#define SAVE_IMAGE_PERIODICALLY

int
main()
{
    using namespace Petrichor;
    using namespace std::chrono_literals;

    constexpr auto kMaxRenderingTime = 123s;
    constexpr auto kSaveInterval = 10s; // 画像を定期的に保存する時間間隔

    Core::Petrichor petrichor;
    petrichor.Initialize();

#ifdef SAVE_IMAGE_PERIODICALLY
    std::thread saveImage([&] {
        uint32_t idxImg      = 0;
        const auto timeBegin = std::chrono::high_resolution_clock::now();
        for (;;)
        {
            std::this_thread::sleep_for(kSaveInterval);
            std::stringstream path;
            path << "Output/" << std::setfill('0') << std::setw(4) << std::right
                 << idxImg++ << ".png";
            petrichor.SaveImage(path.str());
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
                std::chrono::duration_cast<std::chrono::seconds>(now - timeBegin) > (kMaxRenderingTime - 5s);
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

    petrichor.Render();
    petrichor.SaveImage("Output/Result.png");
    petrichor.Finalize();

    return 0;
}
