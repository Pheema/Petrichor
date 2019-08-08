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

Texture2D
IntelOpenImageDenoiser::Denoise(const Texture2D& colorTexture, bool isHDR)
{
    oidn::FilterRef filter = m_device.newFilter("RT");

    // 入力画像を指定する引数がvoid*なのでしぶしぶconst_cast
    // "color"指定ならcolorTextureのデータも書き換わらないはず
    filter.setImage("color",
                    const_cast<Texture2D&>(colorTexture).GetRawDataPtr(),
                    oidn::Format::Float3,
                    colorTexture.GetWidth(),
                    colorTexture.GetHeight());

    Texture2D outputTexture(colorTexture.GetWidth(), colorTexture.GetHeight());
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
            fmt::print(errorMsg);
        }
    }

    return outputTexture;
}

} // namespace Core
} // namespace Petrichor
