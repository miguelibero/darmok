#pragma once

#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
    class ScriptingAppImpl;

    class ScriptingApp final : public App
    {
    public:
        ScriptingApp();
        ~ScriptingApp();
        void init(const std::vector<std::string>& args) override;
    private:
        std::unique_ptr<ScriptingAppImpl> _impl;
    };
}