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
            const Tile tile = [&]() {
                Tile t;
                t.x = i * m_tileWidth;
                t.y = j * m_tileHeight;
                t.width = (i != numTileX) ? m_tileWidth : remainedWidth;
                t.height = (j != numTileY) ? m_tileHeight : remainedHeight;
                return t;
            }();

            m_tiles.emplace_back(tile);
        }
    }
}

} // namespace Core
} // namespace Petrichor
