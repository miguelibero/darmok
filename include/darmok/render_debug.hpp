#pragma once

#include <darmok/export.h>
#include <bgfx/bgfx.h>
#include <memory>


namespace darmok
{
    class MeshData;
    class Program;

    class DARMOK_EXPORT DebugRenderer final
    {
    public:
        DebugRenderer() noexcept;
        void init() noexcept;
        void shutdown() noexcept;
        void renderMesh(MeshData& meshData, uint8_t color, bgfx::ViewId viewId, bgfx::Encoder& encoder, bool lines) noexcept;
    private:
        std::shared_ptr<Program> _prog;
        bgfx::UniformHandle _hasTexturesUniform;
        bgfx::UniformHandle _colorUniform;
    };
}