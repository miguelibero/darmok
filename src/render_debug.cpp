#include <darmok/render_debug.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include "detail/render_samplers.hpp"

namespace darmok
{
    DebugRenderer::DebugRenderer() noexcept
        : _hasTexturesUniform{ bgfx::kInvalidHandle }
        , _colorUniform{ bgfx::kInvalidHandle }
        , _textureUniform{ bgfx::kInvalidHandle }
    {
    }

    expected<void, std::string> DebugRenderer::init(App& app) noexcept 
    {
        auto result = StandardProgramLoader::load(Program::Standard::Unlit);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        _prog = result.value();
        _textureUniform = bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler);
        _hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
        _colorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);

        Image img{ Colors::white(), app.getAssets().getAllocator() };
		auto texResult = Texture::load(img);
		if (!texResult)
        {
            return unexpected{ std::move(texResult).error() };
        }
        _tex = std::make_unique<Texture>(std::move(texResult).value());
        return {};
    }

    expected<void, std::string> DebugRenderer::shutdown() noexcept
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
        return {};
    }

    const std::shared_ptr<Program>& DebugRenderer::getProgram() noexcept
    {
        return _prog;
    }

    expected<void, std::string> DebugRenderer::renderMesh(const Mesh& mesh, bgfx::ViewId viewId, bgfx::Encoder& encoder, const Color& color, bool lines) noexcept
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
        auto result = mesh.render(encoder);
        if (!result)
        {
            return result;
        }
        encoder.setState(state);
        encoder.submit(viewId, _prog->getHandle());
        return {};
    }

    expected<void, std::string> DebugRenderer::renderMesh(MeshData& meshData, bgfx::ViewId viewId, bgfx::Encoder& encoder, uint8_t debugColor, bool lines) noexcept
    {
        auto color = Colors::debug(debugColor);
        auto meshResult = meshData.createMesh(_prog->getVertexLayout());
        if(!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
		}
        meshData.clear();
        return renderMesh(meshResult.value(), viewId, encoder, color, lines);
    }
}