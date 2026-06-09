#pragma once
#include "LevelData.h"
#include <string>
#include <memory>

namespace BurgerTime
{
    class LevelLoader
    {
    public:
        static std::unique_ptr<LevelData> LoadFromFile(const std::string& filepath, int levelId);

    private:
        static BurgerPieceType ParseBurgerPieceType(const std::string& typeStr);
    };
}