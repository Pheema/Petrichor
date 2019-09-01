#include "MixMaterial.h"

namespace Petrichor
{
namespace Core
{

MixMaterial::MixMaterial(const MaterialBase* mat0,
                         const MaterialBase* mat1,
                         float mix)
  : m_mat0(mat0)
  , m_mat1(mat1)
  , m_mix(mix)
{
}
} // namespace Core
} // namespace Petrichor
