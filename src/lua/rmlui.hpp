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
        LuaRmluiAppComponent(RmluiAppComponent& comp) noexcept;

        static LuaRmluiAppComponent addAppComponent(LuaApp& app) noexcept;

        std::reference_wrapper<RmluiView> getDefaultView() noexcept;
        std::reference_wrapper<RmluiView> getView(const std::string& name) noexcept;
        bool hasView(const std::string& name);
        bool removeView(const std::string& name);

        void loadFont(const std::string& path) noexcept;
        void loadFallbackFont(const std::string& path) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<RmluiAppComponent> _comp;
    };
}