#pragma once

#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
    class ScriptingAppImpl;

    class ScriptingApp final : public App
    {
    public:
        DLLEXPORT ScriptingApp();
        DLLEXPORT ~ScriptingApp();
        void init(const std::vector<std::string>& args) override;
        int shutdown() override;
    protected:
        void updateLogic(float deltaTime) override;
    private:
        std::unique_ptr<ScriptingAppImpl> _impl;
    };
}