#pragma once

#include <darmok/scripting.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace darmok
{
    class ScriptingAppImpl final
    {
    public:
        ScriptingAppImpl();
        void init(App& app, const std::vector<std::string>& args);
        void shutdown() noexcept;

        static OptionalRef<App>& getApp() noexcept;

    private:
        std::unique_ptr<sol::state> _lua;
        static OptionalRef<App> _app;
    };
}