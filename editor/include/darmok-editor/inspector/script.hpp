#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/lua_script.hpp>

namespace darmok::editor
{
    class LuaScriptInspectorEditor final : public EntityComponentObjectEditor<LuaScript>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(LuaScript::Definition& text) noexcept override;
    };

    class LuaScriptRunnerInspectorEditor final : public SceneComponentObjectEditor<LuaScriptRunner>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(LuaScriptRunner::Definition& def) noexcept override;
    };
}