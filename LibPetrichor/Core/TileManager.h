#pragma once

#include "Math/Vector3f.h"
#include <mutex>
#include <queue>

namespace Petrichor
{
namespace Core
{

class TileManager
{
public:
    class Tile
    {
        friend class TileManager;

    public:
        Tile() = default;

        Tile(int width, int height)
          : width(width)
          , height(height)
        {
        }

        int x = 0;      //!< タイル左上のコーナーを示すX座標
        int y = 0;      //!< タイル左上のコーナーを示すY座標
        int width = 0;  //!< タイルの幅
        int height = 0; //!< タイルの高さ
    };

    TileManager(int renderWidth,
                int renderHeight,
                int tileWidth,
                int tileHeight);

    const std::vector<Tile>&
    GetTiles() const
    {
        return m_tiles;
    }

    int
    GetNumTiles() const
    {
        return m_numTiles;
    }

private:
    int m_renderWidth = 0;
    int m_renderHeight = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
    int m_numTiles = 0;

    std::vector<Tile> m_tiles;
};

} // namespace Core
} // namespace Petrichor
