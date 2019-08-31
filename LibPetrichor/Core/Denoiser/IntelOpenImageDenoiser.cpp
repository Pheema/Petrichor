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
IntelOpenImageDenoiser::Denoise(const Texture2D& inputColor,
                                const Texture2D* inputNormal,
                                bool isHDR)
{
    oidn::FilterRef filter = m_device.newFilter("RT");

    // 入力画像を指定する引数がvoid*なのでしぶしぶconst_cast
    // "color"指定ならcolorTextureのデータも書き換わらないはず
    filter.setImage("color",
                    const_cast<Texture2D&>(inputColor).GetRawDataPtr(),
                    oidn::Format::Float3,
                    inputColor.GetWidth(),
                    inputColor.GetHeight());

    if (inputNormal)
    {
        filter.setImage("normal",
                        const_cast<Texture2D&>(*inputNormal).GetRawDataPtr(),
                        oidn::Format::Float3,
                        inputNormal->GetWidth(),
                        inputNormal->GetHeight());
    }

    Texture2D outputTexture(inputColor.GetWidth(), inputColor.GetHeight());
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
