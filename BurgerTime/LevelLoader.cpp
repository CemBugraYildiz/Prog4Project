#include "LevelLoader.h"
#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>

using namespace rapidjson;

namespace BurgerTime
{
    std::unique_ptr<LevelData> LevelLoader::LoadFromFile(const std::string& filepath, int levelId)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Failed to open level file: " << filepath << "\n";
            return nullptr;
        }

        // ============================================
        // PARSE JSON ARRAY
        // ============================================
        IStreamWrapper isw(file);
        Document doc;

        ParseResult ok = doc.ParseStream(isw);

        if (!ok)
        {
            std::cerr << "JSON parse error: " << GetParseError_En(ok.Code())
                << " (offset " << ok.Offset() << ")\n";
            return nullptr;
        }

        if (!doc.IsArray())
        {
            std::cerr << "JSON root must be an array\n";
            return nullptr;
        }

        auto levelData = std::make_unique<LevelData>();
        levelData->levelId = levelId;
        levelData->name = "Level " + std::to_string(levelId);

        // ============================================
        // PARSE EACH OBJECT TYPE
        // ============================================
        for (SizeType i = 0; i < doc.Size(); i++)
        {
            const auto& obj = doc[i];

            if (!obj.HasMember("type") || !obj["type"].IsString())
                continue;

            std::string type = obj["type"].GetString();

            if (!obj.HasMember("positions") || !obj["positions"].IsArray())
                continue;

            const auto& positions = obj["positions"];

            // ============================================
            // PARSE POSITIONS
            // ============================================
            if (type == "platform")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->platforms.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "ladder")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->ladders.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "bun")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->topBuns.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "bottombun")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->bottomBuns.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "lettuce")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->lettuces.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "tomato")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->tomatoes.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "cheese")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->cheeses.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "patty")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->patties.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "plate")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->plates.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
            else if (type == "peter")
            {
                if (positions.Size() > 0 && positions[0].IsArray() && positions[0].Size() == 2)
                {
                    levelData->peterSpawn = glm::ivec2(
                        positions[0][0].GetInt(),
                        positions[0][1].GetInt()
                    );
                }
            }
            else if (type == "peterjr")
            {
                if (positions.Size() > 0 && positions[0].IsArray() && positions[0].Size() == 2)
                {
                    levelData->peterJrSpawn = glm::ivec2(
                        positions[0][0].GetInt(),
                        positions[0][1].GetInt()
                    );
                }
            }
            else if (type == "playerdog")
            {
                if (positions.Size() > 0 && positions[0].IsArray() && positions[0].Size() == 2)
                {
                    levelData->playerDogSpawn = glm::ivec2(
                        positions[0][0].GetInt(),
                        positions[0][1].GetInt()
                    );
                }
            }
            else if (type == "enemy")
            {
                for (SizeType j = 0; j < positions.Size(); j++)
                {
                    const auto& pos = positions[j];
                    if (pos.IsArray() && pos.Size() == 2)
                    {
                        levelData->enemySpawns.emplace_back(
                            pos[0].GetInt(),
                            pos[1].GetInt()
                        );
                    }
                }
            }
        }

        std::cout << "Loaded " << levelData->name << ":\n"
            << "   - Platforms: " << levelData->platforms.size() << "\n"
            << "   - Ladders: " << levelData->ladders.size() << "\n"
            << "   - Burger pieces: " << (levelData->topBuns.size() + levelData->lettuces.size() + levelData->patties.size() + levelData->bottomBuns.size()) << "\n"
            << "   - Enemies: " << levelData->enemySpawns.size() << "\n"
            << "   - Player spawn: (" << levelData->peterSpawn.x << ", " << levelData->peterSpawn.y << ")\n";

        return levelData;
    }

    BurgerPieceType LevelLoader::ParseBurgerPieceType(const std::string& typeStr)
    {
        if (typeStr == "bun") return BurgerPieceType::TOP_BUN;
        if (typeStr == "bottombun") return BurgerPieceType::BOTTOM_BUN;
        if (typeStr == "lettuce") return BurgerPieceType::LETTUCE;
        if (typeStr == "tomato") return BurgerPieceType::TOMATO;
        if (typeStr == "cheese") return BurgerPieceType::CHEESE;
        if (typeStr == "patty") return BurgerPieceType::PATTY;

        return BurgerPieceType::PATTY;
    }
}