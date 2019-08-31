#include "IntelOpenImageDenoiser.h"

#include <fmt/format.h>

namespace Petrichor
{
namespace Core
{

IntelOpenImageDenoiser::IntelOpenImageDenoiser()
{
    m_device.commit();
}

Petrichor::Core::Texture2D
IntelOpenImageDenoiser::Denoise(const Texture2D& color, bool isHDR)
{
    oidn::FilterRef filter = m_device.newFilter("RT");

    filter.setImage("color",
                    const_cast<Texture2D&>(color).GetRawDataPtr(),
                    oidn::Format::Float3,
                    color.GetWidth(),
                    color.GetHeight());

    Texture2D outputTexture(color.GetWidth(), color.GetHeight());
    filter.setImage("output",
                    outputTexture.GetRawDataPtr(),
                    oidn::Format::Float3,
                    outputTexture.GetWidth(),
                    outputTexture.GetHeight());
    filter.set("hdr", isHDR);

    filter.commit();

    filter.execute();

    {
        const char* errorMsg = nullptr;
        if (m_device.getError(errorMsg) != oidn::Error::None)
        {
            fmt::print("{}\n", errorMsg);
        }
    }

    return outputTexture;
}

Petrichor::Core::Texture2D
IntelOpenImageDenoiser::Denoise(const Texture2D& color,
                                const Texture2D& albedo,
                                const Texture2D& normal,
                                bool isHDR)
{
    oidn::FilterRef filter = m_device.newFilter("RT");

    filter.setImage("color",
                    const_cast<Texture2D&>(color).GetRawDataPtr(),
                    oidn::Format::Float3,
                    color.GetWidth(),
                    color.GetHeight());

    filter.setImage("albedo",
                    const_cast<Texture2D&>(albedo).GetRawDataPtr(),
                    oidn::Format::Float3,
                    albedo.GetWidth(),
                    albedo.GetHeight());

    filter.setImage("normal",
                    const_cast<Texture2D&>(normal).GetRawDataPtr(),
                    oidn::Format::Float3,
                    normal.GetWidth(),
                    normal.GetHeight());

    Texture2D outputTexture(color.GetWidth(), color.GetHeight());
    filter.setImage("output",
                    outputTexture.GetRawDataPtr(),
                    oidn::Format::Float3,
                    outputTexture.GetWidth(),
                    outputTexture.GetHeight());
    filter.set("hdr", isHDR);
    filter.commit();

    filter.execute();

    {
        const char* errorMsg = nullptr;
        if (m_device.getError(errorMsg) != oidn::Error::None)
        {
            fmt::print("{}\n", errorMsg);
        }
    }

    return outputTexture;
}

} // namespace Core
} // namespace Petrichor
