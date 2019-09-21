#pragma once

#include "Core/Texture2D.h"
#include "OpenImageDenoise/oidn.hpp"

namespace Petrichor
{
namespace Core
{

class IntelOpenImageDenoiser
{
public:
    struct ImageType
    {
        enum Type
        {
            Color,
            Albedo,
            Normal
        };
    };

public:
    IntelOpenImageDenoiser();

    Texture2D
    Denoise(const Texture2D& color, bool isHDR);

    Texture2D
    Denoise(const Texture2D& color,
            const Texture2D& albedo,
            const Texture2D& normal,
            bool isHDR);

private:
    oidn::DeviceRef m_device = oidn::newDevice();
};

} // namespace Core
} // namespace Petrichor
