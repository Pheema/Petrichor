#include "MatMix.h"

namespace Petrichor
{
namespace Core
{

MatMix::MatMix(
    const MaterialBase* mat0,
    const MaterialBase* mat1,
    float mix
) :
    m_mat0(mat0),
    m_mat1(mat1),
    m_mix(mix)
{
}

Petrichor::Core::MaterialTypes MatMix::GetMaterialType(const MaterialBase** mat0 /*= nullptr*/, const MaterialBase** mat1 /*= nullptr*/, float* mix /*= nullptr*/) const
{
    *mat0 = m_mat0;
    *mat1 = m_mat1;
    *mix = m_mix;
    return MaterialTypes::Mix;
}

}
}
