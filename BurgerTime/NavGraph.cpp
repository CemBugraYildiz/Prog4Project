#include "NavGraph.h"
#include "LevelData.h"
#include "LevelManager.h"
#include "GameConfig.h"
#include <algorithm>
#include <queue>
#include <map>
#include <limits>

namespace BurgerTime
{
    void NavGraph::Clear()
    {
        m_Sections.clear();
        m_Graph.clear();
    }

    void NavGraph::Build(const LevelData& data)
    {
        Clear();

        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float enemyW = Config::ENEMY_WIDTH;

        // Group platform tiles by Y build contiguous sections
        std::map<float, std::vector<float>> tilesByY;
        for (const auto& plat : data.platforms)
        {
            auto sp = LevelManager::GridToScreen(plat);
            tilesByY[sp.y].push_back(sp.x);
        }
        for (auto& [y, xs] : tilesByY)
            std::sort(xs.begin(), xs.end());

        int nextId = 0;
        for (const auto& [surfY, xs] : tilesByY)
        {
            int i = 0;
            while (i < (int)xs.size())
            {
                float xMin = xs[i];
                float xMax = xs[i] + tileSize;
                while (i + 1 < (int)xs.size() && xs[i + 1] <= xMax + 1.0f)
                {
                    ++i;
                    xMax = xs[i] + tileSize;
                }
                m_Sections.push_back({ nextId++, surfY, xMin - tol, xMax + tol });
                ++i;
            }
        }

        // Group ladder tiles by X connect sections via edges
        std::map<int, std::vector<int>> byX;
        for (const auto& lad : data.ladders)
            byX[lad.x].push_back(lad.y);
        for (auto& [x, ys] : byX)
            std::sort(ys.begin(), ys.end());

        for (const auto& [gridX, gridYs] : byX)
        {
            float screenX = static_cast<float>(gridX) + Config::LEVEL_OFFSET_X;
            float snapX = screenX + (tileSize - enemyW) * 0.5f;

            int i = 0;
            while (i < (int)gridYs.size())
            {
                int segEnd = i;
                while (segEnd + 1 < (int)gridYs.size() &&
                    gridYs[segEnd + 1] == gridYs[segEnd] + (int)tileSize)
                    ++segEnd;

                float segTopY = static_cast<float>(gridYs[i]);
                float segBotY = static_cast<float>(gridYs[segEnd]) + tileSize;

                std::vector<int> sects;
                for (const auto& sec : m_Sections)
                {
                    if (sec.surfaceY < segTopY - tol || sec.surfaceY > segBotY + tol) continue;
                    if (snapX < sec.xMin || snapX > sec.xMax) continue;
                    sects.push_back(sec.id);
                }

                for (int a = 0; a < (int)sects.size(); ++a)
                    for (int b = a + 1; b < (int)sects.size(); ++b)
                    {
                        m_Graph[sects[a]].push_back({ snapX, sects[b] });
                        m_Graph[sects[b]].push_back({ snapX, sects[a] });
                    }

                i = segEnd + 1;
            }
        }
    }

    std::vector<NavGraph::NavEdge> NavGraph::FindPath(int from, int to) const
    {
        if (from < 0 || to < 0 || from == to) return {};

        std::map<int, int> parent;
        std::map<int, NavEdge> edgeUsed;
        std::queue<int> q;

        parent[from] = from;
        q.push(from);

        while (!q.empty())
        {
            int cur = q.front(); q.pop();
            if (cur == to)
            {
                std::vector<NavEdge> path;
                int node = cur;
                while (node != from)
                {
                    path.push_back(edgeUsed[node]);
                    node = parent[node];
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            auto it = m_Graph.find(cur);
            if (it == m_Graph.end()) continue;

            for (const auto& edge : it->second)
            {
                if (parent.find(edge.destSectionId) == parent.end())
                {
                    parent[edge.destSectionId] = cur;
                    edgeUsed[edge.destSectionId] = edge;
                    q.push(edge.destSectionId);
                }
            }
        }
        return {};
    }

    int NavGraph::GetEntitySection(float x, float y) const
    {
        const float feetY = y + Config::PLAYER_HEIGHT;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE * 2.0f;
        for (const auto& sec : m_Sections)
        {
            if (std::abs(feetY - sec.surfaceY) > tol) continue;
            if (x >= sec.xMin && x <= sec.xMax) return sec.id;
        }
        return -1;
    }

    const NavGraph::PlatSection* NavGraph::GetSection(int id) const
    {
        if (id < 0 || id >= (int)m_Sections.size()) return nullptr;
        return &m_Sections[id];
    }

    int NavGraph::GetNearestSection(float x, float y) const
    {
        const float feetY = y + Config::PLAYER_HEIGHT;
        const float xTol = static_cast<float>(Config::TILE_SIZE);
        int   bestId = -1;
        float bestDist = std::numeric_limits<float>::max();

        for (const auto& sec : m_Sections)
        {
            if (x < sec.xMin - xTol || x > sec.xMax + xTol) continue;
            float dist = std::abs(feetY - sec.surfaceY);
            if (dist < bestDist) { bestDist = dist; bestId = sec.id; }
        }
        return bestId;
    }

    int NavGraph::GetNearestSectionExcluding(float x, float y, int excludeId) const
    {
        const float feetY = y + Config::PLAYER_HEIGHT;
        const float xTol = static_cast<float>(Config::TILE_SIZE);
        int   bestId = -1;
        float bestDist = std::numeric_limits<float>::max();

        for (const auto& sec : m_Sections)
        {
            if (sec.id == excludeId) continue;
            if (x < sec.xMin - xTol || x > sec.xMax + xTol) continue;
            float dist = std::abs(feetY - sec.surfaceY);
            if (dist < bestDist) { bestDist = dist; bestId = sec.id; }
        }
        return bestId;
    }
}