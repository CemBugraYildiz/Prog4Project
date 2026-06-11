#pragma once
#include <Component.h>
#include <array>
#include <string>

namespace BurgerTime
{
    class Player;

    enum class IngredientType { TOP_BUN, BOTTOM_BUN, LETTUCE, TOMATO, CHEESE, PATTY };

    class BurgerPiece final : public dae::Component
    {
    public:
        explicit BurgerPiece(dae::GameObject* owner, IngredientType type);

        void OnAttach() override;
        void Update() override;
        void Render() const override;

        void OnPlayerWalkOver(int segmentIndex);
        bool IsFullyWalked() const;
        bool IsLanded()  const { return m_IsLanded; }
        bool IsFalling() const { return m_IsFalling; } 
        void StartFalling();

    private:
        IngredientType m_Type;
        std::array<bool, 4> m_SegmentWalked{ false, false, false, false };
        int   m_WalkedCount{ 0 };
        bool  m_IsFalling{ false };
        bool  m_IsLanded{ false };
        bool  m_TargetIsPlate{ false };               
        float m_FallSpeed{ 150.0f };
        float m_TargetY{ 0.0f };
        dae::GameObject* m_SegmentObjects[4]{ nullptr };
        int m_EnemiesCrushedThisFall{ 0 };

        void  CreateSegments();
        void  UpdateSegmentVisuals();
        void  CheckPlayerWalkOver(Player* player);
        void  TriggerChainReaction(float landedY);
        float CalculatePlateStackY() const;
        std::string GetSpriteBaseName() const;
        float GetNextPlatformY(bool& outIsPlate) const;
    };
}