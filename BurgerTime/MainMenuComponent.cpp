#include "MainMenuComponent.h"
#include "GameManager.h"
#include "TextComponent.h"
#include "GameMode.h"
#include <algorithm>

namespace BurgerTime
{
    MainMenuComponent::MainMenuComponent(dae::GameObject* owner) : Component(owner) {}

    void MainMenuComponent::AddOption(dae::TextComponent* normal, dae::TextComponent* selected)
    {
        m_Options.push_back({ normal, selected });
    }

    void MainMenuComponent::Navigate(int delta)
    {
        m_Selected = (m_Selected + delta + (int)m_Options.size()) % (int)m_Options.size();
        UpdateVisuals();
    }

    void MainMenuComponent::Confirm()
    {
        switch (m_Selected)
        {
        case 0: GameManager::GetInstance().StartGame(1, GameMode::SinglePlayer); break;
        case 1: GameManager::GetInstance().StartGame(1, GameMode::CoOp); break;
        case 2: GameManager::GetInstance().StartGame(1, GameMode::Versus); break;
        }
    }

    void MainMenuComponent::UpdateVisuals()
    {
        for (int i = 0; i < (int)m_Options.size(); ++i)
        {
            bool sel = (i == m_Selected);
            if (m_Options[i].normal)   m_Options[i].normal->SetActive(!sel);
            if (m_Options[i].selected) m_Options[i].selected->SetActive(sel);
        }
    }
}