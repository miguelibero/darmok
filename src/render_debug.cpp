#include <darmok/render_debug.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include "render_samplers.hpp"

namespace darmok
{
    DebugRenderer::DebugRenderer() noexcept
        : _hasTexturesUniform{ bgfx::kInvalidHandle }
        , _colorUniform{ bgfx::kInvalidHandle }
        , _textureUniform{ bgfx::kInvalidHandle }
    {
    }

    void DebugRenderer::init(App& app) noexcept
    {
        _prog = std::make_shared<Program>(StandardProgramType::Unlit);
        _textureUniform = bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler);
        _hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
        _colorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);

        Image img(Colors::white(), app.getAssets().getAllocator());
        _tex = std::make_unique<Texture>(img);
    }

    void DebugRenderer::shutdown() noexcept
    {
        _prog.reset();
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms =
        { _hasTexturesUniform, _colorUniform, _textureUniform };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform.get()))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
    }

    const std::shared_ptr<Program>& DebugRenderer::getProgram() noexcept
    {
        return _prog;
    }

    void DebugRenderer::renderMesh(IMesh& mesh, bgfx::ViewId viewId, bgfx::Encoder& encoder, const Color& color, bool lines) noexcept
    {
        static const glm::vec4 noTextures(0);
        encoder.setUniform(_hasTexturesUniform, glm::value_ptr(noTextures));
        encoder.setUniform(_colorUniform, glm::value_ptr(Colors::normalize(color)));
        encoder.setTexture(RenderSamplers::MATERIAL_ALBEDO, _textureUniform, _tex->getHandle());
        uint64_t state = BGFX_STATE_DEFAULT;
        if (lines)
        {
            state |= BGFX_STATE_PT_LINES;
        }
        else
        {
            state &= ~BGFX_STATE_CULL_MASK;
        }
        mesh.render(encoder);
        encoder.setState(state);
        encoder.submit(viewId, _prog->getHandle());
    }

    void DebugRenderer::renderMesh(MeshData& meshData, bgfx::ViewId viewId, bgfx::Encoder& encoder, uint8_t debugColor, bool lines) noexcept
    {
        auto color = Colors::debug(debugColor);
        auto mesh = meshData.createMesh(_prog->getVertexLayout());
        meshData.clear();
        renderMesh(*mesh, viewId, encoder, color, lines);
    }
}