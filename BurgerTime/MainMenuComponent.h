#pragma once
#include <Component.h>
#include <vector>

namespace dae { class TextComponent; }

namespace BurgerTime
{
    class MainMenuComponent final : public dae::Component
    {
    public:
        explicit MainMenuComponent(dae::GameObject* owner);
        void Update() override {}

        void AddOption(dae::TextComponent* normal, dae::TextComponent* selected);
        void Navigate(int delta);
        void Confirm();
        void UpdateVisuals();

    private:
        struct Option { dae::TextComponent* normal; dae::TextComponent* selected; };
        std::vector<Option> m_Options;
        int m_Selected{ 0 };
    };
}
