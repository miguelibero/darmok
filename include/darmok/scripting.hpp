#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
    class ScriptingAppImpl;

    class DARMOK_EXPORT ScriptingApp final : public App
    {
    public:
        ScriptingApp();
        ~ScriptingApp();
        void init(const std::vector<std::string>& args) override;
        int shutdown() override;
    protected:
        void updateLogic(float deltaTime) override;
    private:
        std::unique_ptr<ScriptingAppImpl> _impl;
    };
}