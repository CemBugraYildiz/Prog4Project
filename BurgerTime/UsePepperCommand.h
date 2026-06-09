#pragma once
#include "Command.h"
#include "GameObject.h"
#include "Player.h"

namespace BurgerTime
{
    class UsePepperCommand final : public dae::Command
    {
    public:
        explicit UsePepperCommand(dae::GameObject* gameObject)
            : m_pGameObject(gameObject)
        {
        }

        void Execute() override
        {
            if (!m_pGameObject) return;

            auto* player = m_pGameObject->GetComponent<BurgerTime::Player>();
            if (player)
            {
                player->UsePepper();
            }
        }

    private:
        dae::GameObject* m_pGameObject{};
    };
}
