#pragma once

#include <darmok/scripting.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>

#define SOL_ALL_SAFETIES_ON 1
#define SOL_LUAJIT 1
#include <sol/sol.hpp>

namespace darmok
{
    class ScriptingAppImpl final
    {
    public:
        void init(App& app, const std::vector<std::string>& args);
        void updateLogic(float deltaTime);
        void shutdown() noexcept;

    private:
        std::unique_ptr<sol::state> _lua;
        sol::function _luaUpdate;
    };
}