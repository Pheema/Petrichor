#include "TileManager.h"

namespace Petrichor
{
namespace Core
{

TileManager::TileManager(int renderWidth,
                         int renderHeight,
                         int tileWidth,
                         int tileHeight)
  : m_renderWidth(renderWidth)
  , m_renderHeight(renderHeight)
  , m_tileWidth(tileWidth)
  , m_tileHeight(tileHeight)
  , m_numTiles(0)
{
    // 正方ピクセルの個数
    const int numTileX = m_renderWidth / m_tileWidth;
    const int numTileY = m_renderHeight / m_tileHeight;
    m_numTiles = (numTileX + 1) * (numTileY + 1);

    // 画面端に生じる非正方ピクセルの幅、高さ
    const int remainedWidth = m_renderWidth - numTileX * m_tileWidth;
    const int remainedHeight = m_renderHeight - numTileY * m_tileHeight;

    for (int j = 0; j < numTileY + 1; j++)
    {
        for (int i = 0; i < numTileX + 1; i++)
        {
            Tile tile;
            tile.SetInitPixel(i * m_tileWidth, j * m_tileHeight);

            int tileWidth = m_tileWidth;
            int tileHeight = m_tileHeight;
            if (i == numTileX)
            {
                tileWidth = remainedWidth;
            }

            if (j == numTileY)
            {
                tileHeight = remainedHeight;
            }

            tile.SetSize(tileWidth, tileHeight);

            m_tiles.push(tile);
        }
    }
}

bool
TileManager::IsEmpty() const
{
    std::lock_guard lock(m_mtx);
    bool isEmpty = m_tiles.empty();
    return isEmpty;
}

Tile
TileManager::GetTile()
{
    std::lock_guard lock(m_mtx);
    const Tile tile = m_tiles.front();
    m_tiles.pop();
    return tile;
}

} // namespace Core
} // namespace Petrichor
