#pragma once

namespace BurgerTime
{
    namespace Config
    {
        constexpr int WINDOW_WIDTH = 640;
        constexpr int WINDOW_HEIGHT = 754; 

        // Level scope
        constexpr int LEVEL_WIDTH = WINDOW_WIDTH;
        constexpr int LEVEL_HEIGHT = WINDOW_HEIGHT;

        // Grid offset
        constexpr float LEVEL_OFFSET_X = -32.0f;
        constexpr float LEVEL_OFFSET_Y = 0.0f;

        // Platform
        constexpr float PLATFORM_WIDTH = 0.0f;
        constexpr float PLATFORM_HEIGHT = 0.0f;

        // Ladder
        constexpr float LADDER_WIDTH = 64.0f;
        constexpr float LADDER_HEIGHT = 64.0f;

        // Burger Ingredients
        constexpr float BURGER_PIECE_WIDTH = 64.0f;
        constexpr float BURGER_PIECE_HEIGHT = 16.0f;
        constexpr float BURGER_SEGMENT_WIDTH = 16.0f;

        constexpr float INGREDIENT_PLATFORM_OFFSET = 16.0f;

        constexpr float BURGER_MAX_FALL_Y =
            static_cast<float>(WINDOW_HEIGHT) - BURGER_PIECE_HEIGHT;

        // Player
        constexpr float PLAYER_WIDTH = 32.0f;
        constexpr float PLAYER_HEIGHT = 32.0f;
        constexpr float PLAYER_SPEED = 80.0f;
        constexpr float PLAYER_CLIMB_SPEED = 60.0f;

        // Collision tolerance
        constexpr float PLATFORM_SNAP_TOLERANCE = 8.0f;
        constexpr float LADDER_X_TOLERANCE = 2.0f; 

        // Grid
        constexpr int TILE_SIZE = 64;

        // Enemy
        constexpr float ENEMY_WIDTH = 64.0f;
        constexpr float ENEMY_HEIGHT = 64.0f;

        // Plate
        constexpr float PLATE_WIDTH = 75.0f;
        constexpr float PLATE_HEIGHT = 9.0f;
    }
}