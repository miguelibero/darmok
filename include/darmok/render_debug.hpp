#pragma once

#include <darmok/export.h>
#include <darmok/color_fwd.hpp>
#include <darmok/texture.hpp>
#include <bgfx/bgfx.h>
#include <memory>


namespace darmok
{
    class Mesh;
    class MeshData;
    class Program;
    class App;

    class DARMOK_EXPORT DebugRenderer final
    {
    public:
        DebugRenderer() noexcept;
        void init(App& app) noexcept;
        void shutdown() noexcept;
        void renderMesh(MeshData& meshData, bgfx::ViewId viewId, bgfx::Encoder& encoder, uint8_t color, bool lines = true) noexcept;
        void renderMesh(Mesh& mesh, bgfx::ViewId viewId, bgfx::Encoder& encoder, const Color& color = Color(255), bool lines = true) noexcept;
        const std::shared_ptr<Program>& getProgram() noexcept;
    private:
        std::unique_ptr<Texture> _tex;
        std::shared_ptr<Program> _prog;
        bgfx::UniformHandle _textureUniform;
        bgfx::UniformHandle _hasTexturesUniform;
        bgfx::UniformHandle _colorUniform;
    };
}