#include <darmok/environment.hpp>
#include <darmok/camera.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/color.hpp>

#include "generated/shaders/skybox.program.h"
#include "generated/shaders/grid.program.h"

namespace darmok
{
    SkyboxRenderer::SkyboxRenderer(const std::shared_ptr<Texture>& texture) noexcept
        : _texture{ texture }
    {
    }

    expected<void, std::string> SkyboxRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        auto progResult = Program::loadStaticMem(skybox_program);
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        _program = std::make_unique<Program>(std::move(progResult).value());
        _texUniform = { "s_texColor", bgfx::UniformType::Sampler };

        static const Cube screen{ glm::uvec3{ 2 } };
        auto meshResult = MeshData{ screen }.createMesh(_program->getVertexLayout());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
		}
        _mesh = std::make_unique<Mesh>(std::move(meshResult).value());
        return {};
    }

    expected<void, std::string> SkyboxRenderer::shutdown() noexcept
    {
        _texUniform.reset();
        _mesh.reset();
        _program.reset();
        _cam.reset();

        return {};
    }

    expected<void, std::string> SkyboxRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_mesh || !_program || !_texture)
        {
            return unexpected<std::string>{"not initialized"};
        }

        auto renderResult = _mesh->render(encoder);
        if(!renderResult)
        {
            return unexpected{ std::move(renderResult).error() };
		}

        encoder.setTexture(0, _texUniform, _texture->getHandle());

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            ;
        encoder.setState(state);
        encoder.submit(viewId, _program->getHandle());

        return {};
    }

    GridRenderer::GridRenderer(const Config& config) noexcept
        : _config{ config }
    {
    }

    expected<void, std::string> GridRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        auto progResult = Program::loadStaticMem(grid_program);
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        _program = std::make_unique<Program>(std::move(progResult).value());

        _color1Uniform = { "u_gridColor1", bgfx::UniformType::Vec4 };
        _color2Uniform = { "u_gridColor2", bgfx::UniformType::Vec4 };
        _dataUniform = { "u_data", bgfx::UniformType::Vec4 };

        static const Rectangle rect{ glm::vec2{2.0f} };
        MeshData meshData{ rect };
        auto meshResult = meshData.createMesh(_program->getVertexLayout());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }
        _mesh = std::make_unique<Mesh>(std::move(meshResult).value());
        _cam = cam;
        return {};
    }

    expected<void, std::string> GridRenderer::shutdown() noexcept
    {
        _program.reset();
        _mesh.reset();
        _cam.reset();
        _color1Uniform.reset();
        _color2Uniform.reset();
        _dataUniform.reset();
        return {};
    }

    expected<void, std::string> GridRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_mesh || !_program)
        {
            return unexpected<std::string>{"uninitialized"};
        }

        auto result = _mesh->render(encoder);
        if (!result)
        {
            return result;
        }
        auto& proj = _cam->getProjectionMatrix();
        auto depthRange = Math::projDepthRange(proj);
        glm::vec4 data{ depthRange, _config.grids[0].separation, _config.grids[1].separation };

        encoder.setUniform(_color1Uniform, glm::value_ptr(Colors::normalize(_config.grids[0].color)));
        encoder.setUniform(_color2Uniform, glm::value_ptr(Colors::normalize(_config.grids[1].color)));
        encoder.setUniform(_dataUniform, glm::value_ptr(data));

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_BLEND_ALPHA
            | BGFX_STATE_DEPTH_TEST_LESS
            ;
        encoder.setState(state);
        encoder.submit(viewId, _program->getHandle());
        return {};
    }
}