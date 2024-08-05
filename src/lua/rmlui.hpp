#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>

namespace Rml
{
    class Variant;
}

namespace darmok
{
    class Texture;
    class RmluiAppComponent;
    class RmluiView;

    class LuaRmluiView final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;

        static void createDataModel(RmluiView& view, const std::string& name, sol::table table) noexcept;

        static void setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept;
        static void getRmlVariant(const Rml::Variant& variant, sol::table table, sol::object key) noexcept;
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