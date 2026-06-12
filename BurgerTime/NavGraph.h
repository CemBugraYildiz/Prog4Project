#pragma once
#include <vector>
#include <map>

namespace BurgerTime
{
    struct LevelData;

    class NavGraph
    {
    public:
        struct PlatSection
        {
            int   id;
            float surfaceY;
            float xMin;
            float xMax;
        };

        struct NavEdge
        {
            float ladderX;
            int   destSectionId;
        };

        void Build(const LevelData& data);
        void Clear();

        std::vector<NavEdge> FindPath(int from, int to) const;
        int GetEntitySection(float x, float y) const;
        int GetNearestSection(float x, float y) const;
        int GetNearestSectionExcluding(float x, float y, int excludeId) const;
        const PlatSection* GetSection(int id) const;

    private:
        std::vector<PlatSection> m_Sections;
        std::map<int, std::vector<NavEdge>> m_Graph;
    };
}
