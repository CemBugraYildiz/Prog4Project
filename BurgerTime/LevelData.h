#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace BurgerTime
{
    // BURGER PIECE TYPE
    enum class BurgerPieceType
    {
        TOP_BUN,
        BOTTOM_BUN,
        LETTUCE,
        TOMATO,
        CHEESE,
        PATTY
    };

    // ENTITY TYPE
    enum class EntityType
    {
        PETER,          // Player 1
        PETER_JR,       // Player 2 (Coop)
        PLAYER_DOG,     // Versus mode enemy player
        ENEMY           // AI enemy
    };

    // TILE-BASED LEVEL DATA
    struct LevelData
    {
        int levelId;
        std::string name;
        int tileSize{ 64 }; // Grid tile size (64x64 pixels)

        // Tile positions (grid-based)
        std::vector<glm::ivec2> platforms;
        std::vector<glm::ivec2> ladders;

        // Burger pieces (by type)
        std::vector<glm::ivec2> topBuns;
        std::vector<glm::ivec2> bottomBuns;
        std::vector<glm::ivec2> lettuces;
        std::vector<glm::ivec2> tomatoes;
        std::vector<glm::ivec2> cheeses;
        std::vector<glm::ivec2> patties;

        // Plates (burger landing zones)
        std::vector<glm::ivec2> plates;

        // Entities
        glm::ivec2 peterSpawn{ 0, 0 };      // Player 1
        glm::ivec2 peterJrSpawn{ 0, 0 };    // Player 2 (Coop)
        glm::ivec2 playerDogSpawn{ 0, 0 };  // Versus mode
        std::vector<glm::ivec2> enemySpawns;
    };
}