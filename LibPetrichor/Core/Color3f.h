#pragma once

#include "Math/Vector3f.h"

namespace Petrichor
{

using Color3f = Math::Vector3f;

float
GetLuminance(const Color3f color3f);

} // namespace Petrichor
