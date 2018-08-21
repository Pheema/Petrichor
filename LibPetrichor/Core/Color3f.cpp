#include "Color3f.h"

namespace Petrichor
{

float
Petrichor::GetLuminance(const Color3f color3f)
{
    // #TODO: とりあえず。
    return 0.2126f * color3f.x + 0.7152f * color3f.y + 0.0722f * color3f.z;
}

} // namespace Petrichor
