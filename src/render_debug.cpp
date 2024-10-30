#include <darmok/render_debug.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>

namespace darmok
{
    DebugRenderer::DebugRenderer() noexcept
        : _hasTexturesUniform{ bgfx::kInvalidHandle }
        , _colorUniform{ bgfx::kInvalidHandle }
    {
    }

    void DebugRenderer::init() noexcept
    {
        _prog = std::make_shared<Program>(StandardProgramType::Unlit);
        _hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
        _colorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
    }

    void DebugRenderer::shutdown() noexcept
    {
        _prog.reset();
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms = { _hasTexturesUniform, _colorUniform };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform.get()))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
    }

    void DebugRenderer::renderMesh(MeshData& meshData, uint8_t debugColor, bgfx::ViewId viewId, bgfx::Encoder& encoder, bool lines) noexcept
    {
        static const glm::vec4 noTextures(0);
        encoder.setUniform(_hasTexturesUniform, glm::value_ptr(noTextures));
        auto color = Colors::normalize(Colors::debug(debugColor));
        encoder.setUniform(_colorUniform, glm::value_ptr(color));
        uint64_t state = BGFX_STATE_DEFAULT;
        if (lines)
        {
            state |= BGFX_STATE_PT_LINES;
        }
        else
        {
            state &= ~BGFX_STATE_CULL_MASK;
        }
        auto mesh = meshData.createMesh(_prog->getVertexLayout());
        meshData.clear();
        mesh->render(encoder);
        encoder.setState(state);
        encoder.submit(viewId, _prog->getHandle());
    }
}