#pragma once

#include "Core/Texture2D.h"
#include <OpenImageDenoise/oidn.hpp>

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

    //! デノイズを実行する

    Texture2D
    Denoise(const Texture2D& inputColor,
            const Texture2D* inputNormal,
            bool isHDR);

private:
    oidn::DeviceRef m_device = oidn::newDevice();
};

} // namespace Core
} // namespace Petrichor
