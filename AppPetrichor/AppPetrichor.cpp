#include "Core/Denoiser/IntelOpenImageDenoiser.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Logger.h"
#include "Core/Petrichor.h"
#include "TestScene/TestScene.h"
#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <fstream>
#include <gflags/gflags.h>
#include <thread>

DEFINE_string(imageOutputDir, "./RenderedImage", "image output path");
DEFINE_bool(useFixedFilename, false, "use fixed file name.");
DEFINE_uint32(timeLimit, 0, "Rendering time limit");
DEFINE_string(renderSetting, "settings.json", "Render setting file path.");
DEFINE_string(assetSetting, "assets.json", "Asset setting file path.");

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

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

int
main(int argc, char** argv)
{
    using namespace std::chrono_literals;
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::filesystem::path outputDir(
      std::filesystem::weakly_canonical(FLAGS_imageOutputDir));
    if (!std::filesystem::exists(outputDir))
    {
        std::filesystem::create_directory(outputDir);
    }

    std::string filenamePrefix;
    if (!FLAGS_useFixedFilename)
    {
        filenamePrefix += GetCurrentTimeString();
        filenamePrefix += "_";
    }

    {
        std::string logFileName = filenamePrefix + "log.txt";
        Petrichor::Core::Logger::AddConsoleOutput();
        Petrichor::Core::Logger::AddFileOutput(outputDir / logFileName);
    }

    SCOPE_LOGGER(__FUNCTION__);

    Petrichor::Core::Scene scene;
    scene.LoadRenderSetting(
      std::filesystem::weakly_canonical(FLAGS_renderSetting));
    scene.LoadAssets(std::filesystem::weakly_canonical(FLAGS_assetSetting));

    // #TODO: Render()の引数にAOVType(AOVFlag?)を渡してレンダリングする？
    auto targetTexture = std::make_unique<Petrichor::Core::Texture2D>(
      scene.GetRenderSetting().outputWidth,
      scene.GetRenderSetting().outputHeight);

    scene.SetTargetTexture(Petrichor::Core::Scene::AOVType::Rendered,
                           targetTexture.get());

    /*auto uvCoordinateTexture = std::make_unique<Petrichor::Core::Texture2D>(
      scene.GetRenderSetting().outputWidth,
      scene.GetRenderSetting().outputHeight);

    scene.SetTargetTexture(Petrichor::Core::Scene::AOVType::UV,
                           uvCoordinateTexture.get());*/

    scene.SetTargetTexture(Petrichor::Core::Scene::AOVType::Rendered,
                           targetTexture.get());

    auto denoisingAlbedoTexture = std::make_unique<Petrichor::Core::Texture2D>(
      targetTexture->GetWidth(), targetTexture->GetHeight());

    scene.SetTargetTexture(Petrichor::Core::Scene::AOVType::DenoisingAlbedo,
                           denoisingAlbedoTexture.get());

    auto denoisingNormalTexture = std::make_unique<Petrichor::Core::Texture2D>(
      targetTexture->GetWidth(), targetTexture->GetHeight());

    scene.SetTargetTexture(Petrichor::Core::Scene::AOVType::DenoisingNormal,
                           denoisingNormalTexture.get());

    Petrichor::Core::Petrichor petrichor;
    petrichor.SetRenderCallback(nullptr);

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
                      scene.GetTargetTexture(
                        Petrichor::Core::Scene::AOVType::Rendered);
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

    if (targetTexture)
    {
        /*{
            const std::string filename = prefix + ".png";
            targetTexture->Save(outputDir / filename);
        }*/

        Petrichor::Core::IntelOpenImageDenoiser denoiser;
        const Petrichor::Core::Texture2D denoised =
          denoiser.Denoise(*targetTexture,
                           *denoisingAlbedoTexture,
                           *denoisingNormalTexture,
                           true);

        {
            const std::string denoisedFilename = filenamePrefix + "final.png";
            denoised.Save(outputDir / denoisedFilename);
        }
    }

    /*if (uvCoordinateTexture)
    {
        const std::string fileName = filenamePrefix + "uv.png";
        uvCoordinateTexture->Save(outputDir / fileName);
    }

    if (denoisingAlbedoTexture)
    {
        const std::string fileName = filenamePrefix + "_denoisingAlbedo.png";
        denoisingAlbedoTexture->Save(outputDir / fileName);
    }

    if (denoisingNormalTexture)
    {
        const std::string denoisingNormalFileName =
          filenamePrefix + "_denoisingNormal.png";
        denoisingNormalTexture->Save(outputDir / denoisingNormalFileName);
    }*/

    showProgress.join();

    return 0;
}
