#include <Core/Petrichor.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>

// #define ENABLE_TIME_LIMITATION
#define SAVE_IMAGE_PERIODICALLY

int
main()
{
    using namespace Petrichor;
    using namespace std::chrono_literals;

    const auto kTimeLimit = 123s;
    const auto timeBegin  = std::chrono::high_resolution_clock::now();

    Core::Petrichor petrichor;
    petrichor.Initialize();
    std::thread jobRender([&] {
        petrichor.Render();
        petrichor.Finalize();
        exit(0);
    });
    jobRender.detach();

#ifdef SAVE_IMAGE_PERIODICALLY
    std::thread jobSaveImg([&] {
        uint32_t idxImg = 0;
        for (;;)
        {
            std::this_thread::sleep_for(10s);
            std::stringstream path;
            path << "Output/" << std::setfill('0') << std::setw(4) << std::right
                 << idxImg++ << ".png";
            petrichor.SaveImage(path.str());
        }
    });
    jobSaveImg.detach();
#endif

    // 時間オーバー監視
    for (;;)
    {
        const auto timeNow  = std::chrono::high_resolution_clock::now();
        const auto duration = timeNow - timeBegin;

        if (duration > kTimeLimit)
        {
#ifdef ENABLE_TIME_LIMITATION
            petrichor.Finalize();
            exit(0);
#endif
        }
        std::this_thread::sleep_for(1s);
    }

    return 0;
}
