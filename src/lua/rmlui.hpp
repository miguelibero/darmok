#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Texture;
    class RmluiAppComponent;
    class RmluiView;

    class LuaRmluiView final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    };

    class LuaApp;

    class LuaRmluiAppComponent final
    {
    public:

        static void bind(sol::state_view& lua) noexcept;
    private:
        static RmluiAppComponent& addAppComponent(LuaApp& app) noexcept;

        static void loadFont(const std::string& path) noexcept;
        static void loadFallbackFont(const std::string& path) noexcept;
    };
}