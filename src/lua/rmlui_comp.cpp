#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <darmok/rmlui.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include "utils.hpp"

namespace darmok
{
    LuaRmluiRenderer::LuaRmluiRenderer(RmluiRenderer& comp) noexcept
        : _comp(comp)
    {
    }

    void LuaRmluiRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
    }

    void LuaRmluiRenderer::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
    }

    RmluiRenderer& LuaRmluiRenderer::getReal() noexcept
    {
        return _comp;
    }

    const RmluiRenderer& LuaRmluiRenderer::getReal() const noexcept
    {
        return _comp;
    }

    LuaRmluiRenderer& LuaRmluiRenderer::addCameraComponent(Camera& cam) noexcept
    {
        return cam.addComponent<LuaRmluiRenderer>(cam.addComponent<RmluiRenderer>());
    }

    OptionalRef<LuaRmluiRenderer>::std_t LuaRmluiRenderer::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<LuaRmluiRenderer>();
    }

    void LuaRmluiRenderer::loadFont1(const std::string& path) noexcept
    {
        _comp.loadFont(path);
    }

    void LuaRmluiRenderer::loadFont2(const std::string& path, bool fallback) noexcept
    {
        _comp.loadFont(path, fallback);
    }

    void LuaRmluiRenderer::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiCanvas::bind(lua);
        LuaRmluiEvent::bind(lua);

        lua.new_usertype<LuaRmluiRenderer>("RmluiRenderer", sol::no_constructor,
            "add_camera_component", &LuaRmluiRenderer::addCameraComponent,
            "get_camera_component", &LuaRmluiRenderer::getCameraComponent,
            "load_font", sol::overload(
                &LuaRmluiRenderer::loadFont1,
                &LuaRmluiRenderer::loadFont2
            )
        );
    }
}