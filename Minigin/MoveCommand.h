#pragma once
#include "Command.h"
#include "GameObject.h"
#include "EngineTime.h"
#include <type_traits>

namespace dae
{
    // ============================================
    // BASE MOVE COMMAND
    // ============================================
    class MoveCommand : public Command
    {
    public:
        MoveCommand(GameObject* pGameObject, float dx, float dy)
            : m_pGameObject(pGameObject)
            , m_Dx(dx)
            , m_Dy(dy)
        {
        }

        void Execute() override
        {
            if (!m_pGameObject) return;

            const auto pos = m_pGameObject->GetTransform().GetPosition();
            m_pGameObject->SetPosition(pos.x + m_Dx, pos.y + m_Dy);
        }

    protected:
        GameObject* m_pGameObject{};
        float m_Dx{};
        float m_Dy{};
    };

    // ============================================
    // COMPONENT MOVE COMMAND (Template)
    // ============================================
    template<typename TComponent>
    class ComponentMoveCommand final : public MoveCommand
    {
    public:
        ComponentMoveCommand(GameObject* pGameObject, float dx, float dy)
            : MoveCommand(pGameObject, dx, dy)
        {
        }

        void Execute() override
        {
            if (!m_pGameObject) return;

            auto* component = m_pGameObject->GetComponent<TComponent>();
            if (component)
            {
                component->Move(m_Dx, m_Dy, EngineTime::GetDeltaTime());
            }
            else
            {
                MoveCommand::Execute();
            }
        }
    };
}