#pragma once

#include "Math/Vector3f.h"
#include <mutex>
#include <queue>

namespace Petrichor
{
namespace Core
{

class Tile
{
    friend class TileManager;

public:
    Tile() = default;

    Tile(uint32_t width, uint32_t height)
      : m_width(width)
      , m_height(height)
    {
    }

    std::pair<uint32_t, uint32_t>
    GetInitialPixel() const
    {
        return { m_initPixelX, m_initPixelY };
    }

    uint32_t
    GetWidth() const
    {
        return m_width;
    }
    uint32_t
    GetHeight() const
    {
        return m_height;
    }

private:
    void
    SetInitPixel(int i, int j)
    {
        m_initPixelX = i;
        m_initPixelY = j;
    }
    void
    SetSize(uint32_t width, uint32_t height)
    {
        SetWidth(width);
        SetHeight(height);
    }

    void
    SetWidth(uint32_t width)
    {
        m_width = width;
    }
    void
    SetHeight(uint32_t height)
    {
        m_height = height;
    }

    uint32_t m_initPixelX = 0; //!< タイル左上のコーナーを示すX座標
    uint32_t m_initPixelY = 0; //!< タイル左上のコーナーを示すY座標
    uint32_t m_width = 0;      //!< タイルの幅
    uint32_t m_height = 0;     //!< タイルの高さ
};

class TileManager
{
public:
    TileManager(int renderWidth,
                int renderHeight,
                int tileWidth,
                int tileHeight);

    bool
    IsEmpty() const;

    Tile
    GetTile();

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

    std::queue<Tile> m_tiles;

    mutable std::mutex m_mtx;
};

} // namespace Core
} // namespace Petrichor
