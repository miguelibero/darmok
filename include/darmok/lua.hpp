#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
    class LuaRunnerAppImpl;

    class DARMOK_EXPORT LuaRunnerApp final : public App
    {
    public:
        LuaRunnerApp();
        ~LuaRunnerApp();
        void init(const std::vector<std::string>& args) override;
        int shutdown() override;
    protected:
        void updateLogic(float deltaTime) override;
    private:
        std::unique_ptr<LuaRunnerAppImpl> _impl;
    };
}