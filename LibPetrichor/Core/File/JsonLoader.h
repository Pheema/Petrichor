#pragma once

#include "IFileLoader.h"

namespace Petrichor
{
namespace Core
{

template<class T>
class JsonLoader : public IFileLoader
{
public:
    void
    Load() override = 0;
};

} // namespace Core
} // namespace Petrichor
