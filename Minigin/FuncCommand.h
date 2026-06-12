#pragma once
#include "Command.h"
#include <functional>

namespace dae
{
    class FuncCommand final : public Command
    {
    public:
        explicit FuncCommand(std::function<void()> fn) : m_Fn(std::move(fn)) {}
        void Execute() override { if (m_Fn) m_Fn(); }
    private:
        std::function<void()> m_Fn;
    };
}
